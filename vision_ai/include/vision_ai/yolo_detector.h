//
// Created by chiheb on 31/07/2026.
//

#ifndef DETECT_PERSON_YOLODETECTOR_H
#define DETECT_PERSON_YOLODETECTOR_H
#include <opencv2/opencv.hpp>
#include <onnxruntime_cxx_api.h>
struct Detection {
    cv::Rect box;
    float confidence;
    int classId;
    std::string name;
};

class YoloDetector {
public:
    YoloDetector(const std::string& modelPath);
    std::vector<Detection> detect(const cv::Mat& frame);
private:
    void preprocess(const cv::Mat& frame,std::vector<float>& inputTensor);
    Ort::Env env_;
    Ort::Session session_;
    const std::array<int64_t, 4> input_shape_array = {1, 3, 640, 640};

    // Cached buffers for reuse (optimization)
    std::vector<cv::Rect> cached_boxes_;
    std::vector<float> cached_scores_;
    cv::Mat cached_blob_;


};


#endif //DETECT_PERSON_YOLODETECTOR_H
