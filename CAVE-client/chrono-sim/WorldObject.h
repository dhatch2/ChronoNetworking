#ifndef WORLDOBJECT_H
#define WORLDOBJECT_H

#include "physics/ChBodyEasy.h"
#include <google/protobuf/message.h>

using namespace chrono;

class WorldObject {
public:
    WorldObject(google::protobuf::Message& message);
    ~WorldObject();

    bool update(google::protobuf::Message& message);

    std::string type();

private:
    // Message information describing this world object
    google::protobuf::Reflection *reflection;
    google::protobuf::Descriptor *descriptor;

    // The local simulation system it belongs to
    ChSystem m_system;

    // Maps the field number to the body. Not all fields are represented in this manner
    std::vector<ChBody> bodies;

};

#endif // WORLDOBJECT_H
