#include <iostream>
#include "ChNetworkHandler.h"
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
    ChClientHandler handler("localhost", "8082");
    int connectionNumber = handler.connectionNumber();
    std::cout << "connectionNumber = " << connectionNumber << std::endl;
    handler.beginSend();
    auto HMMWV = generateTestVehicle(initLoc1, initRot1);
    auto message = generateVehicleMessageFromWheeledVehicle(&HMMWV.GetVehicle(), connectionNumber, 0);
    handler.beginListen();
    std::thread pop([&] {
        while (true) {
            std::shared_ptr<google::protobuf::Message> rec = handler.popSimMessage();
            rec->PrintDebugString();
        }
    });
    while (true) {
        handler.pushMessage(message);
        messageFromVector(message.mutable_chassiscom(), initLoc2);
        //message.set_connectionnumber(1);
        //sleep(1);
    }
    pop.join();
    return 0;
}
