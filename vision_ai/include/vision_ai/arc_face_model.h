//
// Created by chiheb on 31/07/2026.
//

#ifndef DETECT_PERSON_ARCFACEMODEL_H
#define DETECT_PERSON_ARCFACEMODEL_H
#include <onnxruntime_cxx_api.h>
#include <opencv2/opencv.hpp>

class ArcFaceModel {
public:
    ArcFaceModel(const std::string& modelPath);
    std::vector<float>    extractEmbedding(const cv::Mat& face);

private:
    void preprocess(const cv::Mat& face,std::vector<float>& tensor);
    Ort::Env env_;
    Ort::Session session_;
    const std::array<int64_t, 4> shape_array {1, 3, 112, 112};
};

#endif //DETECT_PERSON_ARCFACEMODEL_H
