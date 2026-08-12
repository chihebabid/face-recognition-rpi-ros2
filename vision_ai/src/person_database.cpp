//
// Created by chiheb on 31/07/2026.
//
#include <cmath>
#include <filesystem>
#include "vision_ai/person_database.h"

PersonDatabase::PersonDatabase(ArcFaceModel &arc_face_model, const std::string &prefix) : arc_face_model_{arc_face_model}, prefix_{prefix} {
}

void PersonDatabase::addPerson(const std::string &name, const std::vector<float> &embedding) {
    persons_[name] = embedding;
}

float PersonDatabase::cosineSimilarity(const std::vector<float> &a, const std::vector<float> &b) {
    if (a.size() != b.size())
        return -1.0f;

    float dot{}, normA{}, normB{};

    for (size_t i{}; i < a.size(); ++i) {
        dot += a[i] * b[i];
        normA += a[i] * a[i];
        normB += b[i] * b[i];
    }

    normA = std::sqrt(normA);
    normB = std::sqrt(normB);

    if (normA == 0.0f || normB == 0.0f)
        return -1.0f;

    return dot / (normA * normB);
}

std::string PersonDatabase::identify(const std::vector<float> &embedding, float threshold) const {
    float bestScore{-1.0f};
    std::string bestPerson{"Unknown"};

    for (const auto &[name, refEmbedding]: persons_) {
        float score = cosineSimilarity(embedding, refEmbedding);
        if (score > bestScore) {
            bestScore = score;
            bestPerson = name;
        }
    }

    if (bestScore < threshold)
        return "Unknown";

    return bestPerson;
}

/*
 * @brief Load images from files and compute their embedding average
**/

std::vector<float> PersonDatabase::loadFromFileAverageEmbedding(const std::string &name) {
    std::vector<float> averageEmbedding(512, 0.0f);

    std::size_t imageCount{};
    for (const auto &entry: std::filesystem::directory_iterator(name)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        if (entry.path().extension() != ".jpg" && entry.path().extension() != ".png") {
            continue;
        }
        cv::Mat img = cv::imread(entry.path().string());
        auto emb = arc_face_model_.extractEmbedding(img);
        for (size_t i{}; i < emb.size(); ++i) {
            averageEmbedding[i] += emb[i];
        }
        ++imageCount;
    }
    std::for_each(averageEmbedding.begin(), averageEmbedding.end(), [imageCount](float &v) { v /= imageCount; });
    return averageEmbedding;
}


// void PersonDatabase::load() {
//     auto embedding = loadFromFileAverageEmbedding("chiheb");
//     addPerson("Chiheb", embedding);
//
//     embedding = loadFromFileAverageEmbedding("yasmine");
//     addPerson("Yasmine", embedding);
//
//     embedding = loadFromFileAverageEmbedding("chahd");
//     addPerson("Chahd", embedding);
// }

void PersonDatabase::loadFromPrefix() {
    std::cout<<__func__<<'\n';
    std::cout<<"Path: "<<prefix_<<'\n';
    if (!std::filesystem::exists(prefix_)) {
        std::cerr << "Directory does not exist: " << prefix_ << '\n';
        return;
    }
    for (const auto &entry: std::filesystem::directory_iterator(prefix_)) {
        std::cout<<"Checking entry: "<<entry.path().string()<<'\n';
        if (!entry.is_directory()) {
            continue;
        }
        std::cout<<"Loading person: "<<entry.path().filename().string()<<'\n';
        auto embedding = loadFromFileAverageEmbedding(entry.path().string());
        addPerson(entry.path().filename().string(), embedding);
    }
}
