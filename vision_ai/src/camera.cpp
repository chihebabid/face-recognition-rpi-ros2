#include <chrono>
#include <rclcpp/rclcpp.hpp>


#include <pigpiod_if2.h>
#include "cam_detect_sub.h"
#include "core_detector.h"
#include "defs.h"
#include "controlservonode.h"
#include "vision_ai/cam_detect_sub.h"

auto main(int argc, char *argv[]) -> int {
    rclcpp::init(argc, argv);
    cc::pi = pigpio_start(NULL, NULL);
    if (cc::pi < 0) {
        std::cerr << "Error initilizing pigpio library..." << std::endl;
        return cc::pi;
    }

    auto cam_node{std::make_shared<CamDetectSub>("cam_server","models/mobilefacenet.onnx","models/yolov8n_face.onnx")};
    auto servo_node{std::make_shared<ControlServoNode>("servo_server")};

    rclcpp::executors::MultiThreadedExecutor executor;
    executor.add_node(cam_node);
    // executor.add_node(servo_node);
    executor.spin();
    rclcpp::shutdown();
    pigpio_stop(cc::pi);
    return 0;
}
