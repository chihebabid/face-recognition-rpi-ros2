#ifndef SUPERVISESERVO_H
#define SUPERVISESERVO_H
#include <QObject>
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/u_int8.hpp"
#include "std_msgs/msg/string.hpp"

class ControlCAM;
class SuperviseServo : public rclcpp::Node {
    const ControlCAM* mControlCAM;
    std::shared_ptr<rclcpp::Subscription<std_msgs::msg::UInt8>> mSubscriber;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr mPublisher;
    void callback(const std_msgs::msg::UInt8::ConstSharedPtr &msg);
    bool mEnable {false};
public:
    SuperviseServo(const ControlCAM *cam);
    void enableSubscription(const bool &);
};

#endif // SUPERVISESERVO_H
