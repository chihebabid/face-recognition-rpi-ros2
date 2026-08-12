//
// Created by chiheb on 03/08/2026.
//
#include <chrono>
#include "core_detector.h"
#include "../include/vision_ai/core_detector.h"

#include "../include/vision_ai/arc_face_model.h"
#include "../include/vision_ai/yolo_detector.h"
static const cv::Scalar COLOR_GREEN{0, 255, 0};
static constexpr int MIN_HEIGHT = 40;
static constexpr int MIN_WIDTH = 40;
static const int FONT = cv::FONT_HERSHEY_SIMPLEX;
static constexpr float FONT_SCALE = 0.6f;
static constexpr int FONT_THICKNESS = 2;
static constexpr int BOX_THICKNESS = 2;

static constexpr int MIN_Y_TEXT = 20;
static constexpr int TEXT_OFFSET_Y = 10;
CoreDetector::CoreDetector(const std::string &arc_face_model_path, const std::string &yolo_model_path, cv::Mat &frame) : arc_face_{arc_face_model_path},
                                             yolo_detector_{yolo_model_path}, person_db_{arc_face_,"face_db/"}, frame_{frame} {
    person_db_.loadFromPrefix();
}


auto CoreDetector::process() -> void {
     auto temp_detections{yolo_detector_.detect(frame_)};
     {
         std::lock_guard lk{mutex_ldetect_};
         l_detections_=std::move(temp_detections);
     }

    for (auto &det: l_detections_) {
        // Early filter: skip small detections
        if (det.box.width < MIN_WIDTH || det.box.height < MIN_HEIGHT) {
            continue;
        }

        // Extract embedding and identify person
        auto embedding = arc_face_.extractEmbedding(frame_(det.box));
        det.name = person_db_.identify(embedding, 0.60f);

        // Draw bounding box and label
        cv::rectangle(frame_, det.box, COLOR_GREEN, BOX_THICKNESS);
        cv::putText(frame_, det.name, cv::Point(det.box.x, std::max(MIN_Y_TEXT, det.box.y - TEXT_OFFSET_Y)),
                    FONT, FONT_SCALE, COLOR_GREEN, FONT_THICKNESS);
    }
}


void CoreDetector::refresh(cv::Mat &frame) {
    std::lock_guard lk{mutex_ldetect_};
    for (const auto &det: l_detections_) {
        if (det.box.width < MIN_WIDTH || det.box.height < MIN_HEIGHT) {
            continue;
        }
        cv::rectangle(frame, det.box, COLOR_GREEN, BOX_THICKNESS);
        cv::putText(frame, det.name, cv::Point(det.box.x, std::max(MIN_Y_TEXT, det.box.y - TEXT_OFFSET_Y)),
                    FONT, FONT_SCALE, COLOR_GREEN, FONT_THICKNESS);
    }
}
