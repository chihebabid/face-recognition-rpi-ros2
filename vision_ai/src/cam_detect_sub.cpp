//
// Created by chiheb on 04/08/2026.
//

#include "vision_ai/cam_detect_sub.h"
#include <memory>

using namespace std::placeholders;

CamDetectSub::CamDetectSub(const std::string &name, const std::string &arc_path, const std::string &yolo_path)
    : Node(name), detector_(arc_path, yolo_path, m_frame) {

    m_subscriber = create_subscription<std_msgs::msg::String>(
        "cam/param", 10,
        [this](const std_msgs::msg::String::ConstSharedPtr &msg) { my_callback(msg); });

    // QoS SensorData (Best Effort) pour minimiser la latence

    m_publisher = image_transport::create_publisher(*this, "cam/image", rclcpp::SensorDataQoS());
    th_detector_ = std::thread(&CamDetectSub::threadHandler, this);
#ifdef USE_LIBCAMERA
    init_libcamera();
#else
    // Fallback OpenCV avec Timer pour le mode non-libcamera
    m_cap.open(0, cv::CAP_V4L2);
    if (!m_cap.isOpened()) {
        RCLCPP_WARN(this->get_logger(), "Impossible d'ouvrir OpenCV VideoCapture!");
    } else {
        m_cap.set(cv::CAP_PROP_FRAME_WIDTH, 320);
        m_cap.set(cv::CAP_PROP_FRAME_HEIGHT, 240);
    }
    using namespace std::chrono_literals;
    m_timer = create_wall_timer(50ms, [this]() { timer_callback(); });
#endif
}

CamDetectSub::~CamDetectSub() {
#ifdef USE_LIBCAMERA
    stop_libcamera();
#endif
}

#ifdef USE_LIBCAMERA
void CamDetectSub::init_libcamera() {
    cm_ = std::make_unique<libcamera::CameraManager>();
    if (cm_->start() < 0) return;

    if (cm_->cameras().empty()) return;

    camera_ = cm_->cameras()[0];
    camera_->acquire();

    auto config = camera_->generateConfiguration({libcamera::StreamRole::Viewfinder});
    auto &sconfig = config->at(0);

    // RGB888 natif OpenCV (évite la conversion logicielle)
    sconfig.pixelFormat = libcamera::formats::RGB888;
    sconfig.size = {320, 240};
    config->validate();
    camera_->configure(config.get());

    stream_ = sconfig.stream();
    allocator_ = std::make_unique<libcamera::FrameBufferAllocator>(camera_);
    if (allocator_->allocate(stream_) < 0) return;

    // Multi-buffering : Pré-maper tous les tampons attribués par le matériel
    const auto &buffers = allocator_->buffers(stream_);
    for (const auto &buffer : buffers) {
        const auto &plane = buffer->planes()[0];
        void *mem = mmap(NULL, plane.length, PROT_READ | PROT_WRITE, MAP_SHARED, plane.fd.get(), plane.offset);
        if (mem != MAP_FAILED) {
            mapped_buffers_[buffer.get()] = mem;
        }

        auto request = camera_->createRequest();
        if (request) {
            request->addBuffer(stream_, buffer.get());
            requests_.push_back(std::move(request));
        }
    }

    // Connexion événementielle du callback de fin de capture
    camera_->requestCompleted.connect(this, &CamDetectSub::request_complete);

    camera_->start();
    is_active_ = true;

    // Injection initiale de toutes les requêtes dans le pipeline matériel
    for (auto &req : requests_) {
        camera_->queueRequest(req.get());
    }
}

void CamDetectSub::stop_libcamera() {
    if (camera_) {
        is_active_ = false;
        camera_->stop();

        for (auto &[buffer, ptr] : mapped_buffers_) {
            if (ptr) munmap(ptr, buffer->planes()[0].length);
        }
        mapped_buffers_.clear();
        requests_.clear();

        if (allocator_) allocator_->free(stream_);
        camera_->release();
        camera_.reset();
    }
    if (cm_) cm_->stop();
}

// CALLBACK ÉVÉNEMENTIEL (Appelé dès qu'une frame est prête)
void CamDetectSub::request_complete(libcamera::Request *req) {
    if (req->status() == libcamera::Request::RequestCancelled) return;

    if (is_active_) {
        auto buffer = req->buffers().at(stream_);
        void *data = mapped_buffers_[buffer];

        if (data) {
            auto frame {cv::Mat(240, 320, CV_8UC3, data)};
            {
                std::unique_lock lk{mtx_cv_};
                if (!is_frame_available_) {
                    m_frame=frame.clone();
                    is_frame_available_ = true;
                    cv_.notify_one();
                    RCLCPP_INFO(this->get_logger(), "Frame capturée et prête pour le traitement.");
                }
            }

            std_msgs::msg::Header header;
            header.stamp = this->now();
            header.frame_id = "camera";
            std::shared_ptr<sensor_msgs::msg::Image> msg;
            if (!is_frame_available_) {
                msg = cv_bridge::CvImage(header, "bgr8", m_frame).toImageMsg();
            }
            else {
                detector_.refresh(frame);
                msg = cv_bridge::CvImage(header, "bgr8", frame).toImageMsg();

            }
            m_publisher.publish(msg); is_processing_.store(false);
        }
    }

    // Réinjection immédiate de la requête dans la boucle matérielle libcamera
    req->reuse(libcamera::Request::ReuseBuffers);
    camera_->queueRequest(req);
}
#endif

void CamDetectSub::my_callback(const std_msgs::msg::String::ConstSharedPtr &msg) {
    if (msg->data == "ON") {
#ifdef USE_LIBCAMERA
        is_active_ = true;
#else
        if (m_timer && m_timer->is_canceled()) {
            if (!m_cap.isOpened()) {
                m_cap.open(0, cv::CAP_V4L2);
                m_cap.set(cv::CAP_PROP_FRAME_WIDTH, 320);
                m_cap.set(cv::CAP_PROP_FRAME_HEIGHT, 240);
            }
            m_timer->reset();
        }
#endif
    } else {
#ifdef USE_LIBCAMERA
        is_active_ = false;
#else
        if (m_timer) m_timer->cancel();
        m_cap.release();
#endif
    }
}

#ifndef USE_LIBCAMERA
void CamDetectSub::timer_callback() {
    if (m_cap.isOpened()) {
        m_cap >> m_frame;
        if (!m_frame.empty()) {
            static int i = 0;
            if (i == 0) {
                detector_.process();
            } else {
                detector_.refresh();
            }
            i = (i + 1) % 3;

            std_msgs::msg::Header header;
            header.stamp = this->now();
            header.frame_id = "camera";

            auto msg = cv_bridge::CvImage(header, "bgr8", m_frame).toImageMsg();
            m_publisher.publish(msg);
        }
    }
}
#endif


void CamDetectSub::threadHandler() {
    while (!is_finished_) {
        {
            std::unique_lock lk{mtx_cv_};
            while (!is_frame_available_ && !is_finished_) {
                cv_.wait(lk);
            }
        }

        if (!is_finished_) {
            detector_.process();
        }

        {
            std::unique_lock lk{mtx_cv_};
            is_frame_available_=false;
        }
    }
}