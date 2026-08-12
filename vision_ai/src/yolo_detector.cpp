#include "vision_ai/yolo_detector.h"

#include <opencv2/dnn.hpp>
#include <iostream>
#include <algorithm>
#include <chrono>


constexpr int HEIGHT {640};
constexpr int WIDTH {640};

// Helper function to create optimized SessionOptions
static Ort::SessionOptions createOptimizedSessionOptions() {
    Ort::SessionOptions sessionOptions;

    // 1. Enable maximum graph optimizations (Constant folding, node fusion, etc.)
    sessionOptions.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);

    // 2. Limit to 2 or 3 threads (Leaves CPU headroom for ROS 2 & libcamera)
    sessionOptions.SetIntraOpNumThreads(3);

    // 3. Force sequential execution (Prevents ONNX from spawning extra thread pools)
    sessionOptions.SetInterOpNumThreads(1);
    sessionOptions.SetExecutionMode(ExecutionMode::ORT_SEQUENTIAL);

    // 4. Reuse memory allocations for fixed-size tensors
    // sessionOptions.se.SetMemPattern(true);

    return sessionOptions;
}

YoloDetector::YoloDetector(const std::string &modelPath)
    : env_(ORT_LOGGING_LEVEL_WARNING, "yolo"),
      session_(env_, modelPath.c_str(), createOptimizedSessionOptions()) {

    // Pre-allocate cached buffers to avoid repeated allocations
    cached_boxes_.reserve(100);  // Adjust based on expected detections per frame
    cached_scores_.reserve(100);

    std::cout << "YOLO model loaded with optimized configuration: " << modelPath << std::endl;
}

std::vector<Detection> YoloDetector::detect(const cv::Mat &frame) {
    //------------------------------------------------------
    // Preprocess
    //------------------------------------------------------
    std::vector<Detection> detections;
    static std::vector<float> inputTensor(3*WIDTH*HEIGHT);
    preprocess(frame, inputTensor);

    static Ort::MemoryInfo memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    Ort::Value input = Ort::Value::CreateTensor<float>(memoryInfo, inputTensor.data(), inputTensor.size(), input_shape_array.data(),
                                                       input_shape_array.size());

    static Ort::AllocatorWithDefaultOptions allocator;
    static auto inputName = session_.GetInputNameAllocated(0, allocator);
    static auto outputName = session_.GetOutputNameAllocated(0, allocator);

    static const char *inputNames[] = {inputName.get()};
    static const char *outputNames[] = {outputName.get()};

    // Inference
    auto outputs = session_.Run(Ort::RunOptions{nullptr}, inputNames, &input, 1, outputNames, 1);

    // Output tensor
    const float *data = outputs[0].GetTensorData<float>();
    auto tensorInfo = outputs[0].GetTensorTypeAndShapeInfo();
    auto shape = tensorInfo.GetShape();

    if (shape.size() != 3) {
        std::cerr << "Unexpected output shape" << std::endl;
        return detections;
    }

    const int numPredictions = static_cast<int>(shape[2]);

    // Decode (using cached buffers to avoid repeated allocations)
    cached_boxes_.clear();
    cached_scores_.clear();

    float scaleX = frame.cols / static_cast<float>(WIDTH);
    float scaleY = frame.rows / static_cast<float>(HEIGHT);
    for (int i{}; i < numPredictions; ++i) {
        float score {data[4 * numPredictions + i]};
        if (score < 0.50f) {
            continue;
        }

        float cx = data[i];
        float cy = data[numPredictions + i];
        float w = data[2 * numPredictions + i];
        float h = data[3 * numPredictions + i];
        float left = cx - w * 0.5f;
        float top = cy - h * 0.5f;

        cv::Rect box;
        box.x = static_cast<int>(left * scaleX);
        box.y = static_cast<int>(top * scaleY);
        box.width = static_cast<int>(w * scaleX);
        box.height = static_cast<int>(h * scaleY);
        cached_boxes_.push_back(box);
        cached_scores_.push_back(score);
    }

    std::vector<int> indices;
    cv::dnn::NMSBoxes(cached_boxes_, cached_scores_, 0.50f, 0.45f, indices);


    detections.reserve(indices.size());  // Pre-allocate result vector
    for (int idx: indices) {
        detections.emplace_back(Detection{cached_boxes_[idx], cached_scores_[idx], 0, "Unknown"});
    }

    return detections;
}


void YoloDetector::preprocess(const cv::Mat &frame, std::vector<float> &tensor) {
    cv::dnn::blobFromImage(frame, cached_blob_, 1.f/255.f, cv::Size(WIDTH, HEIGHT), cv::Scalar(), /*swapRB=*/true, /*crop=*/false);
    constexpr auto expected = static_cast<size_t>(3 * WIDTH * HEIGHT);
    const float *blobData = cached_blob_.ptr<float>(0);
    std::memcpy(tensor.data(), blobData, expected * sizeof(float));
}
