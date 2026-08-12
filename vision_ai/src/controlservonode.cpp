//
// Created by chiheb on 13/04/24.
//
#include <chrono>
#include <pigpiod_if2.h>
#include "controlservonode.h"
#include "defs.h"
using namespace std::chrono_literals;

ControlServoNode::ControlServoNode(const std::string & nom): Node(nom) {
    declare_parameter<int>("servo_pin",13);
    get_parameter("servo_pin",cc::servo_pin);

    mSubscriber = create_subscription<std_msgs::msg::UInt8>(
            "cam/param/servo/angle",
            10,
            [this](const std_msgs::msg::UInt8::ConstSharedPtr &msg){
                my_callback(msg);
            }
    );

    mSubscriberControl = create_subscription<std_msgs::msg::String>(
            "cam/param/servo/control",
            10,
            [this](const std_msgs::msg::String::ConstSharedPtr &msg){
                if (msg->data=="ON" && !mAutomatic) {
                    mSubscriber.reset();
                    mAutomatic=true;
                    mPublisher=create_publisher<std_msgs::msg::UInt8>("cam/param/servo/angle",10);
                    mTimer=create_wall_timer(500ms,[this](){
                        rotate();
                    });

                }
                else if (msg->data=="OFF" && mAutomatic) {
                    mAutomatic=false;
                    mTimer->cancel();
                    mPublisher.reset();
                    mPublisher=nullptr;
                    mTimer= nullptr;
                    mSubscriber = create_subscription<std_msgs::msg::UInt8>(
                            "cam/param/servo/angle",
                            10,
                            [this](const std_msgs::msg::UInt8::ConstSharedPtr &msg){
                                my_callback(msg);
                            }
                    );
                }
            }
    );
}

void ControlServoNode::my_callback(const std_msgs::msg::UInt8::ConstSharedPtr &msg) {
    if (!mAutomatic) {
        mCurrentAngle = msg->data;
        uint16_t pulse_width = ((2450 - 1000) * mCurrentAngle / 180) + 1000;
        set_servo_pulsewidth(cc::pi, cc::servo_pin, pulse_width);
        std::this_thread::sleep_for(50ms);
        set_servo_pulsewidth(cc::pi, cc::servo_pin, 0);
    }
}

void ControlServoNode::rotate() {
    static int8_t step {1};
    if (mCurrentAngle+step>180 || mCurrentAngle+step<0) step=-step;
    mCurrentAngle=mCurrentAngle+step;
    std_msgs::msg::UInt8 msg;
    msg.data=mCurrentAngle;
    mPublisher->publish(msg);
    uint16_t pulse_width = ((2450 - 1000) * mCurrentAngle / 180) + 1000;
    set_servo_pulsewidth(cc::pi, cc::servo_pin, pulse_width);
    /*std::this_thread::sleep_for(50ms);
    set_servo_pulsewidth(cc::pi, cc::servo_pin, 0);*/
}
