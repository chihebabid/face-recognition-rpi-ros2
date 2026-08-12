//
// Created by chiheb on 04/08/2026.
//

#ifndef WS_ROS_CAMDETECT_H
#define WS_ROS_CAMDETECT_H
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <cv_bridge/cv_bridge.hpp>
#include <image_transport/image_transport.hpp>
#include <sensor_msgs/msg/compressed_image.hpp>
#include "vision_ai/core_detector.h"
#include "vision_ai/person_database.h"
#include "libcamera/libcamera.h"

#define USE_LIBCAMERA

#ifdef USE_LIBCAMERA
#include <libcamera/libcamera.h>
#include <sys/mman.h>
#include <mutex>
#include <condition_variable>
#endif

class CamDetectSub : public rclcpp::Node {
public:
    CamDetectSub(const std::string &name, const std::string &arc_path, const std::string &yolo_path);
    ~CamDetectSub() override;

private:
    void my_callback(const std_msgs::msg::String::ConstSharedPtr &msg);
    void timer_callback();
    bool capture_frame();

    cv::Mat m_frame;
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr m_subscriber;
    image_transport::Publisher m_publisher;
    rclcpp::TimerBase::SharedPtr m_timer;
    CoreDetector detector_;
#ifdef USE_LIBCAMERA
    bool is_active_{};
    std::unique_ptr<libcamera::CameraManager> cm_;
    std::shared_ptr<libcamera::Camera> camera_;
    libcamera::Stream *stream_{nullptr};
    std::unique_ptr<libcamera::FrameBufferAllocator> allocator_;
    std::vector<std::unique_ptr<libcamera::Request>> requests_;
    std::unordered_map<const libcamera::FrameBuffer*, void*> mapped_buffers_;
    size_t mapped_len_{0};

    libcamera::Request *completed_req_{nullptr};

    bool is_frame_available_{};
    std::atomic<bool> is_finished_{};
    std::atomic<bool> is_processing_{};
    std::mutex mtx_cv_;
    std::condition_variable cv_;
    std::thread th_detector_;

    void threadHandler();
    void init_libcamera();
    void stop_libcamera();
    void request_complete(libcamera::Request *req);
#else
    cv::VideoCapture m_cap;
#endif
};



// class CamDetectSub : public rclcpp::Node {
// public:
//     CamDetectSub(const std::string& name,const std::string &arc_path,const std::string& yolo_path);
//     ~CamDetectSub() {}
// private:
//     void my_callback(const std_msgs::msg::String::ConstSharedPtr &msg);
//     void timer_callback() ;
//
//     cv::Mat m_frame;
//     std::shared_ptr<rclcpp::Subscription<std_msgs::msg::String> > m_subscriber;
//     cv::VideoCapture m_cap;
//     image_transport::Publisher m_publisher;
//     std::shared_ptr<rclcpp::TimerBase> m_timer;
//     CoreDetector detector_;
//     std::unique_ptr<libcamera::CameraManager> cm_;
// };


#endif //WS_ROS_CAMDETECT_H
