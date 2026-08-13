#include "superviseservo.h"
#include "controlcam.h"
SuperviseServo::SuperviseServo(const ControlCAM *cam):Node("supervise_servo"),mControlCAM(cam) {
    mSubscriber=create_subscription<std_msgs::msg::UInt8>(
        "cam/param/servo/angle", 10,[this](const std_msgs::msg::UInt8::ConstSharedPtr &msg) {
            callback(msg);
        });
     mPublisher=create_publisher<std_msgs::msg::String>("cam/param/servo/control",10);
}


void SuperviseServo::callback(const std_msgs::msg::UInt8::ConstSharedPtr &msg) {
    if (mEnable) {
        mControlCAM->notifyNewAngle(msg->data);
    }
}


void SuperviseServo::enableSubscription(const bool &b) {
    mEnable=b;
    std_msgs::msg::String msg;
    msg.data=mEnable ? "ON" : "OFF";
    mPublisher->publish(msg);
}

