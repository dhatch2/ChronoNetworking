#include <iostream>

#include "World.h"
#include "ChronoMessages.pb.h"
#include "MessageConversions.h"

#include "chrono/core/ChFileutils.h"
#include "chrono/core/ChStream.h"
#include "chrono/core/ChRealtimeStep.h"
#include "chrono/physics/ChSystem.h"
#include "chrono/utils/ChUtilsInputOutput.h"

#include "chrono_vehicle/ChConfigVehicle.h"
#include "chrono_vehicle/ChVehicleModelData.h"
#include "chrono_vehicle/terrain/RigidTerrain.h"
//#include "chrono_vehicle/driver/ChIrrGuiDriver.h"
#include "chrono_vehicle/driver/ChDataDriver.h"
//#include "chrono_vehicle/wheeled_vehicle/utils/ChWheeledVehicleIrrApp.h"

#include "chrono_models/vehicle/hmmwv/HMMWV.h"

using namespace chrono;
using namespace chrono::vehicle;
using namespace chrono::vehicle::hmmwv;

enum DriverMode { DEFAULT, RECORD, PLAYBACK };
DriverMode driver_mode = DEFAULT;

// Type of powertrain model (SHAFTS, SIMPLE)
PowertrainModelType powertrain_model = PowertrainModelType::SHAFTS;

// Drive type (FWD, RWD, or AWD)
DrivelineType drive_type = DrivelineType::AWD;

// Type of tire model (RIGID, RIGID_MESH, PACEJKA, LUGRE, FIALA)
//TireModelType tire_model = TireModelType::RIGID;

// Rigid terrain
RigidTerrain::Type terrain_model = RigidTerrain::FLAT;
bool terrain_vis = true;
double terrainHeight = 0;      // terrain height (FLAT terrain only)
double terrainLength = 1000.0;  // size in X direction
double terrainWidth = 1000.0;   // size in Y direction

// Point on chassis tracked by the camera
ChVector<> trackPoint(0.0, 0.0, 1.75);

// Contact method
ChMaterialSurface::ContactMethod contact_method = ChMaterialSurface::SMC;
bool contact_vis = false;

// Simulation step sizes
double step_size = 1e-3;
double tire_step_size = step_size;

// Simulation end time
double t_end = 1000;

// Time interval between two render frames
double render_step_size = 1.0 / 50;  // FPS = 50

// Debug logging
bool debug_output = false;
double debug_step_size = 1.0 / 1;  // FPS = 1

// POV-Ray output
bool povray_output = false;

ChVector<> initLoc1(0, 0, 1.6);
ChQuaternion<> initRot1(1, 0, 0, 0);

ChVector<> initLoc2(1, 1, 1.6);
ChQuaternion<> initRot2(0, 1, 0, 0);

HMMWV_Full generateTestVehicle(ChVector<> initLoc, ChQuaternion<> initRot) {
    HMMWV_Full my_hmmwv; // Test vehicle for messages

    my_hmmwv.SetContactMethod(contact_method);
    my_hmmwv.SetChassisFixed(false);
    my_hmmwv.SetInitPosition(ChCoordsys<>(initLoc, initRot));
    my_hmmwv.SetPowertrainType(powertrain_model);
    my_hmmwv.SetDriveType(drive_type);
    //my_hmmwv.SetTireType(tire_model);
    my_hmmwv.SetTireStepSize(tire_step_size);
    my_hmmwv.SetPacejkaParamfile("hmmwv/tire/HMMWV_pacejka.tir");
    my_hmmwv.Initialize();

    return my_hmmwv;
}

int main(int argc, char **argv) {
    World world;
    bool added = world.registerConnectionNumber(0);
    bool fakeAdded = world.registerConnectionNumber(0);
    if (added && !fakeAdded) {
        std::cout << "PASSED -- World test 1" << std::endl;
    } else std::cout << "FAILED -- World test 1" << std::endl;

    boost::asio::io_service ioService;
    boost::asio::ip::udp::resolver udpResolver(ioService);
    boost::asio::ip::udp::resolver::query udpQuery(boost::asio::ip::udp::v4(), "localhost", "8082");
    boost::asio::ip::udp::endpoint serverEndpoint = *udpResolver.resolve(udpQuery);

    bool addedEndpoint = world.registerEndpoint(serverEndpoint, 0);
    bool fakeAddedEndpoint = world.registerEndpoint(serverEndpoint, 0);
    if (addedEndpoint && !fakeAddedEndpoint) {
        std::cout << "PASSED -- World test 2" << std::endl;
    } else std::cout << "FAILED -- World test 2" << std::endl;

    ChronoMessages::VehicleMessage vehicle = generateVehicleMessageFromWheeledVehicle(&generateTestVehicle(initLoc1, initRot1).GetVehicle(), 0, 0);
    auto vehiclePtr = std::make_shared<ChronoMessages::VehicleMessage>();
    *vehiclePtr = vehicle;

    endpointProfile *profile = world.verifyConnection(0, serverEndpoint);
    bool updated = world.updateElement(vehiclePtr, profile, 0);
    if (profile != NULL && updated) {
        std::cout << "PASSED -- World test 3" << std::endl;
    } else std::cout << "FAILED -- World test 3" << std::endl;

    endpointProfile *fakeProfile = world.verifyConnection(1, serverEndpoint);
    if (fakeProfile == NULL) {
        std::cout << "PASSED -- World test 4" << '\n';
    } else std::cout << "FAILED -- World test 4" << '\n';

    world.registerConnectionNumber(1);
    world.registerConnectionNumber(2);
    world.registerConnectionNumber(3);
    world.registerConnectionNumber(4);
    world.registerConnectionNumber(5);

    world.registerEndpoint(serverEndpoint, 1);
    endpointProfile *profile1 = world.verifyConnection(1, serverEndpoint);
    bool updateOnce = world.updateElement(vehiclePtr, profile1, 0);
    bool updateTwice = world.updateElement(vehiclePtr, profile1, 0);
    if (updateOnce && updateTwice && world.elementCount() == 2) {
        std::cout << "PASSED -- World test 5" << '\n';
    } else std::cout << "FAILED -- World test 5" << '\n';

    world.updateElement(vehiclePtr, profile1, 1);
    if (world.elementCount() == 3 && world.connectionCount() == 2) {
        std::cout << "PASSED -- World test 6" << '\n';
    } else std::cout << "FAILED -- World test 6" << '\n';

    if (world.profileElementCount(profile) == 1 && world.profileElementCount(profile1) == 2) {
        std::cout << "PASSED -- World test 7" << '\n';
    } else std::cout << "FAILED -- World test 7" << '\n';

    bool registered = world.registerEndpoint(serverEndpoint, 2);
    endpointProfile *profile2 = world.verifyConnection(2, serverEndpoint);
    if (registered && profile2 != NULL) {
        std::cout << "PASSED -- World test 8" << '\n';
    } else std::cout << "FAILED -- World test 8" << '\n';

    world.updateElement(vehiclePtr, profile2, 0);
    vehiclePtr->set_idnumber(1);
    world.updateElement(vehiclePtr, profile2, 1);
    vehiclePtr->set_idnumber(2);
    world.updateElement(vehiclePtr, profile2, 2);
    vehiclePtr->set_idnumber(3);
    world.updateElement(vehiclePtr, profile2, 3);
    vehiclePtr->set_idnumber(4);
    world.updateElement(vehiclePtr, profile2, 4);

    if (world.elementCount() == 8 && world.profileElementCount(profile2) == 5 && world.connectionCount() == 3) {
        std::cout << "PASSED -- World test 9" << '\n';
    } else std::cout << "FAILED -- World test 9" << '\n';

    bool removed = world.removeElement(0, profile2);
    if (removed && world.elementCount() == 7 && world.profileElementCount(profile2) == 4) {
        std::cout << "PASSED -- World test 10" << '\n';
    } else std::cout << "FAILED -- World test 10" << '\n';

    if (world.getElement(2, 4)->DebugString().compare(vehiclePtr->DebugString()) == 0) {
        std::cout << "PASSED -- World test 11" << std::endl;
    } else std::cout << "FAILED -- World test 11" << std::endl;

    ChronoMessages::VehicleMessage newVehicle = generateVehicleMessageFromWheeledVehicle(&generateTestVehicle(initLoc2, initRot2).GetVehicle(), 0, 0);

    auto packet = std::make_shared<ChronoMessages::MessagePacket>();
    packet->set_connectionnumber(2);
    packet->add_vehiclemessages();
    packet->mutable_vehiclemessages(0)->CopyFrom(newVehicle);
    newVehicle.set_idnumber(1);
    packet->add_vehiclemessages();
    packet->mutable_vehiclemessages(1)->CopyFrom(newVehicle);
    newVehicle.set_idnumber(2);
    packet->add_vehiclemessages();
    packet->mutable_vehiclemessages(2)->CopyFrom(newVehicle);
    newVehicle.set_idnumber(3);
    packet->add_vehiclemessages();
    packet->mutable_vehiclemessages(3)->CopyFrom(newVehicle);
    newVehicle.set_idnumber(4);
    packet->add_vehiclemessages();
    packet->mutable_vehiclemessages(4)->CopyFrom(newVehicle);

    updated = world.updateElementsOfProfile(profile2, packet);

    if (world.getElement(2, 4)->DebugString().compare(newVehicle.DebugString()) == 0 && packet->vehiclemessages_size() == 0) {
        std::cout << "PASSED -- World test 12" << std::endl;
    } else std::cout << "FAILED -- World test 12" << std::endl;

    packet->add_vehiclemessages();
    packet->mutable_vehiclemessages(0)->CopyFrom(newVehicle);
    newVehicle.set_idnumber(1);
    packet->add_vehiclemessages();
    packet->mutable_vehiclemessages(1)->CopyFrom(newVehicle);
    newVehicle.set_idnumber(2);
    packet->add_vehiclemessages();
    packet->mutable_vehiclemessages(2)->CopyFrom(newVehicle);
    newVehicle.set_idnumber(3);
    packet->add_vehiclemessages();
    packet->mutable_vehiclemessages(3)->CopyFrom(newVehicle);
    newVehicle.set_idnumber(4);
    packet->add_vehiclemessages();
    packet->mutable_vehiclemessages(4)->CopyFrom(newVehicle);
    newVehicle.set_idnumber(5);
    packet->add_vehiclemessages();
    packet->mutable_vehiclemessages(5)->CopyFrom(newVehicle);
    newVehicle.set_idnumber(6);
    packet->add_vehiclemessages();
    packet->mutable_vehiclemessages(6)->CopyFrom(newVehicle);

    world.updateElementsOfProfile(profile2, packet);

    if (world.getElement(2, 6)->DebugString().compare(newVehicle.DebugString()) == 0) {
        std::cout << "PASSED -- World test 13" << std::endl;
    } else std::cout << "FAILED -- World test 13" << std::endl;

    //world.generateWorldPacket()->PrintDebugString();

    removed = world.removeConnection(profile2);
    if (removed && world.elementCount() == 3 && world.connectionCount() == 2) {
        std::cout << "PASSED -- World test 14" << '\n';
    } else std::cout << "FAILED -- World test 14" << '\n';

    world.removeConnection(profile);
    world.removeConnection(profile1);
    if (world.elementCount() == 0 && world.connectionCount() == 0) {
        std::cout << "PASSED -- World test 15" << '\n';
    } else std::cout << "FAILED -- World test 15" << '\n';

    return 0;
}
