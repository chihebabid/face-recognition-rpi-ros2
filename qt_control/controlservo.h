#ifndef CONTROLSERVO_H
#define CONTROLSERVO_H
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/u_int8.hpp"
class ControlServo : public rclcpp::Node {
    rclcpp::Publisher<std_msgs::msg::UInt8>::SharedPtr mPublisher;

public:
    ControlServo();
    void publish(const int& angle);
};

#endif // CONTROLSERVO_H
