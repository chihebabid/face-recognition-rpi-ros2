//
// Created by chiheb on 31/07/2026.
//

#include "vision_ai/arc_face_model.h"

ArcFaceModel::ArcFaceModel(const std::string &modelPath) : env_(ORT_LOGGING_LEVEL_WARNING, "arcface"), session_(env_, modelPath.c_str(),
                                                               Ort::SessionOptions{}) {
}

void ArcFaceModel::preprocess(const cv::Mat &face, std::vector<float> &tensor) {
    cv::Mat resized;
    cv::resize(face, resized, cv::Size(112, 112));
    cv::cvtColor(resized, resized, cv::COLOR_BGR2RGB);
    resized.convertTo(resized, CV_32FC3, 1.0 / 255.0);

    std::vector<cv::Mat> channels;
    cv::split(resized, channels);
    constexpr int channelSize = 112 * 112;


    std::memcpy(tensor.data(), channels[0].ptr<float>(), channelSize * sizeof(float));
    std::memcpy(tensor.data() + channelSize, channels[1].ptr<float>(), channelSize * sizeof(float));
    std::memcpy(tensor.data() + 2 * channelSize, channels[2].ptr<float>(), channelSize * sizeof(float));
}

std::vector<float> ArcFaceModel::extractEmbedding(const cv::Mat &face) {
    std::vector<float> inputTensor(3*112*112);
    preprocess(face, inputTensor);

    Ort::MemoryInfo memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    Ort::Value input = Ort::Value::CreateTensor<float>(memoryInfo, inputTensor.data(), inputTensor.size(), shape_array.data(), shape_array.size());

    Ort::AllocatorWithDefaultOptions allocator;
    auto inputName = session_.GetInputNameAllocated(0, allocator);
    auto outputName = session_.GetOutputNameAllocated(0, allocator);

    const char *inputNames[] = {inputName.get()};
    const char *outputNames[] = {outputName.get()};

    auto outputs =
            session_.Run(Ort::RunOptions{nullptr}, inputNames, &input, 1, outputNames, 1);

    const float *data = outputs[0].GetTensorData<float>();
    auto info = outputs[0].GetTensorTypeAndShapeInfo();
    auto outShape = info.GetShape();

    size_t embeddingSize {static_cast<size_t>(outShape.back())};
    std::vector<float> embedding(embeddingSize);
    std::copy(data, data + embeddingSize, embedding.begin());

    float norm {0.0f};
    std::for_each(embedding.begin(), embedding.end(), [&norm](const float& v) { norm += v * v; });

    if (norm > 0.0f) {
        norm = std::sqrt(norm);
        std::for_each(embedding.begin(), embedding.end(), [norm](float &v) { v /= norm; });
    }

    return embedding;
}
