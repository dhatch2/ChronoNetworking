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

HMMWV_Full generateTestVehicle() {
    HMMWV_Full my_hmmwv; // Test vehicle for messages

    ChVector<> initLoc1(0, 0, 1.6);
    ChQuaternion<> initRot1(1, 0, 0, 0);

    my_hmmwv.SetContactMethod(contact_method);
    my_hmmwv.SetChassisFixed(false);
    my_hmmwv.SetInitPosition(ChCoordsys<>(initLoc1, initRot1));
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

    ChronoMessages::VehicleMessage vehicle = generateVehicleMessageFromWheeledVehicle(&generateTestVehicle().GetVehicle(), 0);
    auto vehiclePtr = std::make_shared<ChronoMessages::VehicleMessage>();
    *vehiclePtr = vehicle;

    endpointProfile *profile = world.verifyConnection(0, serverEndpoint);
    bool updated = world.updateElement(vehiclePtr, profile, 0);
    if (profile != NULL && updated) {
        std::cout << "PASSED -- World test 3" << std::endl;
    } else std::cout << "FAILED -- World test 3" << std::endl;

    return 0;
}
