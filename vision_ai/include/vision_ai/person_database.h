//
// Created by chiheb on 31/07/2026.
//

#ifndef DETECT_PERSON_PERSONDATABASE_H
#define DETECT_PERSON_PERSONDATABASE_H


#include <string>
#include <vector>
#include <unordered_map>
#include "vision_ai/arc_face_model.h"
class PersonDatabase {
public:
    PersonDatabase(ArcFaceModel& arc_face_model,const std::string &prefix="");
    void addPerson(const std::string &name, const std::vector<float> &embedding);
    std::string identify(const std::vector<float> &embedding, float threshold = 0.6f) const;
    //void load();
    void loadFromPrefix();
private:
    std::vector<float> loadFromFileAverageEmbedding(const std::string &name);
    ArcFaceModel& arc_face_model_;
    const std::string prefix_;
    static float cosineSimilarity(const std::vector<float> &a, const std::vector<float> &b);
    std::unordered_map<std::string, std::vector<float> > persons_;
};


#endif //DETECT_PERSON_PERSONDATABASE_H
