#ifndef MESSAGECONVERSIONS_H
#define MESSAGECONVERSIONS_H

#include "ChronoMessages.pb.h"
#include "chrono/physics/ChSystem.h"
#include "chrono_models/vehicle/hmmwv/HMMWV.h"

using namespace chrono;
using namespace chrono::vehicle;

ChronoMessages::VehicleMessage generateVehicleMessageFromWheeledVehicle(ChWheeledVehicle* vehicle, int connectionNumber);
void messageFromVector(ChronoMessages::VehicleMessage_MVector* message,
                       ChVector<> vector);
void messageFromQuaternion(ChronoMessages::VehicleMessage_MQuaternion* message,
                           ChQuaternion<> quaternion);

#endif
