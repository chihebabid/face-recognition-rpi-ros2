#ifndef CONTROLCAM_H
#define CONTROLCAM_H
#include <QObject>
#include <QImage>
#include <QElapsedTimer>
#include <cv_bridge/cv_bridge.hpp>
#include <opencv2/highgui.hpp>
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "sensor_msgs/msg/image.hpp"
#include <image_transport/image_transport.hpp>
#include "controlservo.h"
#include "superviseservo.h"
class ControlCAM :public QObject {
    Q_OBJECT
    QImage mImage;
    rclcpp::Node::SharedPtr mNode;
    std::shared_ptr<ControlServo> mNodeServo;
    std::shared_ptr<SuperviseServo> mNodeSupervise;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr mPublisher;
    std::shared_ptr<image_transport::ImageTransport> mImageTransport;
    image_transport::Subscriber mSubscriber;
    QImage rosImageToQImage(const sensor_msgs::msg::Image::ConstSharedPtr& msg);
    QElapsedTimer timer_;
public:
    ControlCAM();
    void turnON();
    void turnOFF();
    void rotateServo(const int &);
    void notifyNewAngle(const uint8_t &new_angle) const;
    void enableSubscriptionServo(const bool &);
signals:
    void imageLoaded(const QImage &);
    void newAngle(int) const;


};

#endif // CONTROLCAM_H
