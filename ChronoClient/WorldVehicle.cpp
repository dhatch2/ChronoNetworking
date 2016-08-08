#include "WorldVehicle.h"

WorldVehicle::WorldVehicle()
{
    chassis = std::make_shared<ChBody>();
    auto sphere = std::make_shared<ChSphereShape>();
    frontRightWheel = std::make_shared<ChBody>();
    auto frontRightCylinder = std::make_shared<ChCylinderShape>();
    frontLeftWheel = std::make_shared<ChBody>();
    auto frontLeftCylinder = std::make_shared<ChCylinderShape>();
    backRightWheel = std::make_shared<ChBody>();
    auto backRightCylinder = std::make_shared<ChCylinderShape>();
    backLeftWheel = std::make_shared<ChBody>();
    auto backLeftCylinder = std::make_shared<ChCylinderShape>();
    // TODO: Work on making some good cylinder sizes and the blue and white textures.
}

WorldVehicle::~WorldVehicle()
{
}

