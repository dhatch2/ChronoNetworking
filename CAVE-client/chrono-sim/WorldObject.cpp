#include "WorldObject.h"

ChVector<> vectorFromMessage(google::protobuf::message& message) {
    ChVector<> vector;
    auto descriptor = message.GetDescriptor();
    auto reflection = message.GetReflection();
    auto x = descriptor->field(0);
    auto y = descriptor->field(1);
    auto z = descriptor->field(2);
    vector.set(reflection->GetInt32(x), reflection->GetInt32(y), reflection->GetInt32(z));
    return vector;
}

WorldObject::WorldObject(google::protobuf::Message& message) {
    descriptor = message.GetDescriptor();
    reflection = message.GetReflection();
}

bool WorldObject::update(google::protobuf::Message& message) {
    if (descriptor->full_name().compare(message->GetDescriptor()->full_name()) == 0) {
        if (descriptor->full_name().compare(VEHICLE_MESSAGE_TYPE) == 0) {
            // TODO: Set bodies for vehicle case
        }
        return true;
    }
    return false;
}

std::string WorldObject::type() {
    descriptor->full_name();
}
