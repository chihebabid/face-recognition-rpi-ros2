//
// Created by chiheb on 03/08/2026.
//

#ifndef WS_ROS_COREDETECTOR_H
#define WS_ROS_COREDETECTOR_H
#include <string>
#include "vision_ai/yolo_detector.h"
#include "vision_ai/arc_face_model.h"
#include "vision_ai/person_database.h"


class CoreDetector {
public:
    CoreDetector(const std::string & arc_face_model_path,const std::string &yolo_model_path, cv::Mat& frame);
    void process();
    void refresh(cv::Mat &frame);
private:


    ArcFaceModel arc_face_;
    YoloDetector yolo_detector_;
    std::vector<Detection> l_detections_;
    PersonDatabase person_db_;
    cv::Mat& frame_;
    std::mutex mutex_ldetect_;
};


#endif //WS_ROS_COREDETECTOR_H
