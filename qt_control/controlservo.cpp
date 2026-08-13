#include "controlservo.h"

ControlServo::ControlServo():rclcpp::Node("control_servo") {
    mPublisher=create_publisher<std_msgs::msg::UInt8>("cam/param/servo/angle",10);
}


void ControlServo::publish(const int& angle) {
    std_msgs::msg::UInt8 msg;
    msg.data=angle;
    mPublisher->publish(msg);
}

