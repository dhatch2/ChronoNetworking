#ifndef WORLDVEHICLE_H
#define WORLDVEHICLE_H

#include <memory>
#include "chrono/physics/ChSystem.h"
#include "chrono/utils/ChUtilsInputOutput.h"

using namespace chrono;

class WorldVehicle
{
public:
    WorldVehicle();
    ~WorldVehicle();
private:
    std::shared_ptr<ChBody> chassis;
    std::shared_ptr<ChBody> frontRightWheel;
    std::shared_ptr<ChBody> frontLeftWheel;
    std::shared_ptr<ChBody> backRightWheel;
    std::shared_ptr<ChBody> backLeftWheel;
};

#endif // WORLDVEHICLE_H
