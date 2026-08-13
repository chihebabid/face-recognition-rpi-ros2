#include <QDebug>
#include "controlcam.h"

ControlCAM::ControlCAM() {
    mNodeSupervise=std::make_shared<SuperviseServo>(this);
    mNode=rclcpp::Node::make_shared("cam_client");
    mNodeServo=std::make_shared<ControlServo>();
    mPublisher=mNode->create_publisher<std_msgs::msg::String>("cam/param",10);
    // Subscribe to video topic
    image_transport::ImageTransport it(mNode);

    auto qos=rclcpp::SensorDataQoS().get_rmw_qos_profile();
    timer_.start();
    mSubscriber = image_transport::create_subscription(
        mNode.get(), // Pointeur vers le noeud rclcpp::Node
        "cam/image",
        [this](const sensor_msgs::msg::Image::ConstSharedPtr& msg) {
            static int frameCount {};
            ++frameCount;

            if (timer_.elapsed() >= 1000)
            {
                qDebug() << "Display FPS:" << frameCount;

                frameCount = 0;
                timer_.restart();
            }
            mImage = rosImageToQImage(msg);
            emit imageLoaded(mImage);
        },
        "compressed", // Nom du transport ("compressed", "raw", "theora", etc.)
        qos
        );

    std::thread detached_thread{[this](){
        rclcpp::executors::MultiThreadedExecutor executor;
        executor.add_node(mNode);
        executor.add_node(mNodeServo);
        executor.add_node(mNodeSupervise);
        executor.spin();
    }};
    detached_thread.detach();
}

void ControlCAM::turnON() {
    std_msgs::msg::String msg;
    msg.data="ON";
    mPublisher->publish<std_msgs::msg::String>(msg);
}

void ControlCAM::turnOFF() {
    std_msgs::msg::String msg;
    msg.data="OFF";
    mPublisher->publish<std_msgs::msg::String>(msg);
}


QImage ControlCAM::rosImageToQImage(const sensor_msgs::msg::Image::ConstSharedPtr& msg) {
    cv_bridge::CvImagePtr cv_ptr;
    try {
        cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
    } catch (cv_bridge::Exception& e) {
        // Gérer l'erreur de conversion
        return QImage();
    }
    return QImage(cv_ptr->image.data, cv_ptr->image.cols, cv_ptr->image.rows, QImage::Format_RGB888).rgbSwapped();
}

void ControlCAM::rotateServo(const int &angle) {
    mNodeServo->publish(angle);
}


void ControlCAM::notifyNewAngle(const uint8_t &new_angle) const {
    emit newAngle(new_angle);
}

void ControlCAM::enableSubscriptionServo(const bool &b) {
    mNodeSupervise->enableSubscription(b);
}
