//
// Created by chiheb on 13/04/24.
//

#ifndef WS_ROS_PI_CONTROLSERVO_HPP
#define WS_ROS_PI_CONTROLSERVO_HPP
#include <string>
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/u_int8.hpp>
#include <std_msgs/msg/string.hpp>
class ControlServoNode : public rclcpp::Node {
public:
    ControlServoNode(const std::string & nom);

private:
    /**
     * @brief Enable/Disable the camera.
     * @param msg The received ROS string message (ON/OFF
     */
    void my_callback(const std_msgs::msg::UInt8::ConstSharedPtr &msg);

    void rotate();

    std::shared_ptr<rclcpp::Subscription<std_msgs::msg::UInt8>> mSubscriber;
    rclcpp::Publisher<std_msgs::msg::UInt8>::SharedPtr mPublisher;
    std::shared_ptr<rclcpp::Subscription<std_msgs::msg::String>> mSubscriberControl;
    bool mAutomatic {false};
    uint8_t mCurrentAngle;
    rclcpp::TimerBase::SharedPtr mTimer;
};
#endif //WS_ROS_PI_CONTROLSERVO_HPP
