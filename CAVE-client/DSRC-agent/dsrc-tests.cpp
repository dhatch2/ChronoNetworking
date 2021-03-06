// =============================================================================
// PROJECT CHRONO - http://projectchrono.org
//
// Copyright (c) 2014 projectchrono.org
// All right reserved.
//
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file at the top level of the distribution and at
// http://projectchrono.org/license-chrono.txt.
//
// =============================================================================
// Authors: Dylan Hatch
// =============================================================================
//
//	Unit tests for ChDSRCAgent and related DSRC simulation capabilities.
//
// =============================================================================

#include <iostream>
#include "ChDSRCAgent.h"
#include "ChSafeQueue.h"

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

#include <google/protobuf/message.h>

using namespace chrono;
using namespace chrono::vehicle;
using namespace chrono::vehicle::hmmwv;
// =============================================================================

// Initial vehicle location and orientation
ChVector<> initLoc1(0, 0, 1.6);
ChQuaternion<> initRot1(1, 0, 0, 0);
ChVector<> initLoc2(0, 5, 1.6);
ChQuaternion<> initRot2(1, 0, 0, 0);
ChVector<> initLoc3(11, 0, 1.6);
ChQuaternion<> initRot3(1, 0, 0, 0);

// ChQuaternion<> initRot(0.866025, 0, 0, 0.5);
// ChQuaternion<> initRot(0.7071068, 0, 0, 0.7071068);
// ChQuaternion<> initRot(0.25882, 0, 0, 0.965926);
// ChQuaternion<> initRot(0, 0, 0, 1);

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

// =============================================================================

int main(int argc, char* argv[]) {
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    // --------------
    // Create systems
    // --------------
    // Create the HMMWV vehicle, set parameters, and initialize
    HMMWV_Full my_hmmwv1;

    my_hmmwv1.SetContactMethod(contact_method);
    my_hmmwv1.SetChassisFixed(false);
    my_hmmwv1.SetInitPosition(ChCoordsys<>(initLoc1, initRot1));
    my_hmmwv1.SetPowertrainType(powertrain_model);
    my_hmmwv1.SetDriveType(drive_type);
    //my_hmmwv.SetTireType(tire_model);
    my_hmmwv1.SetTireStepSize(tire_step_size);
    my_hmmwv1.SetPacejkaParamfile("hmmwv/tire/HMMWV_pacejka.tir");
    my_hmmwv1.Initialize();

    HMMWV_Full my_hmmwv2;

    my_hmmwv2.SetContactMethod(contact_method);
    my_hmmwv2.SetChassisFixed(false);
    my_hmmwv2.SetInitPosition(ChCoordsys<>(initLoc2, initRot2));
    my_hmmwv2.SetPowertrainType(powertrain_model);
    my_hmmwv2.SetDriveType(drive_type);
    //my_hmmwv2.SetTireType(tire_model);
    my_hmmwv2.SetTireStepSize(tire_step_size);
    my_hmmwv2.SetPacejkaParamfile("hmmwv/tire/HMMWV_pacejka.tir");
    my_hmmwv2.Initialize();

    HMMWV_Full my_hmmwv3;

    my_hmmwv3.SetContactMethod(contact_method);
    my_hmmwv3.SetChassisFixed(false);
    my_hmmwv3.SetInitPosition(ChCoordsys<>(initLoc3, initRot3));
    my_hmmwv3.SetPowertrainType(powertrain_model);
    my_hmmwv3.SetDriveType(drive_type);
    //my_hmmwv3.SetTireType(tire_model);
    my_hmmwv3.SetTireStepSize(tire_step_size);
    my_hmmwv3.SetPacejkaParamfile("hmmwv/tire/HMMWV_pacejka.tir");
    my_hmmwv3.Initialize();

    // Create the terrain
    RigidTerrain terrain(my_hmmwv3.GetSystem());
    terrain.SetContactFrictionCoefficient(0.9f);
    terrain.SetContactRestitutionCoefficient(0.01f);
    terrain.SetContactMaterialProperties(2e7f, 0.3f);
    terrain.SetColor(ChColor(0.8f, 0.8f, 0.5f));
    terrain.EnableVisualization(terrain_vis);
    switch (terrain_model) {
        case RigidTerrain::FLAT:
            terrain.Initialize(terrainHeight, terrainLength, terrainWidth);
            break;
        case RigidTerrain::HEIGHT_MAP:
            terrain.Initialize(vehicle::GetDataFile("terrain/height_maps/test64.bmp"), "test64", 128, 128, 0, 4);
            break;
        case RigidTerrain::MESH:
            terrain.Initialize(vehicle::GetDataFile("terrain/meshes/test.obj"), "test_mesh");
            break;
    }


    // -----------------
    // Initialize output
    // -----------------

    utils::CSV_writer driver_csv(" ");

    // -----------------
    // Unit Tests
    // -----------------

    ChDSRCAgent agent1(&my_hmmwv1.GetVehicle());
    ChDSRCAgent agent2(&my_hmmwv2.GetVehicle());
    ChDSRCAgent agent3(&my_hmmwv3.GetVehicle());

    if(agent1.canReach(agent2.vehicleNumber()))
        std::cout << "PASSED -- Distance test 1" << std::endl;
    else std::cout << "FAILED -- Distance test 1" << std::endl;

    if(!agent1.canReach(agent3.vehicleNumber()))
        std::cout << "PASSED -- Distance test 2" << std::endl;
    else std::cout << "FAILED -- Distance test 2" << std::endl;

    if(!agent2.canReach(agent3.vehicleNumber()))
        std::cout << "PASSED -- Distance test 3" << std::endl;
    else std::cout << "FAILED -- Distance test 3" << std::endl;

    if(agent2.canReach(agent1.vehicleNumber()))
        std::cout << "PASSED -- Distance test 4" << std::endl;
    else std::cout << "FAILED -- Distance test 4" << std::endl;

    std::string message = "Yeeeeea boiiiiiiiiiiii";
    std::vector<uint8_t> messageBuffer(message.size());
    memcpy(messageBuffer.data(), message.c_str(), message.size());
    agent1.broadcastMessage(messageBuffer);
    auto messageBufferRec = agent2.popMessage();
    std::string messageRec((char *)messageBufferRec.data(), messageBufferRec.size());

    if (messageRec.compare(message) == 0)
        std::cout << "PASSED -- Broadcast test 1 " << std::endl; //(" << messageRec << ")" << std::endl;
    else std::cout << "FAILED -- Broadcast test 1 (" << messageRec << ")" << std::endl;

    if (agent3.popMessage().size() == 0)
        std::cout << "PASSED -- Broadcast test 2" << std::endl;
    else std::cout << "FAILED -- Broadcast test 2" << std::endl;

    std::string message1 = "Yeeeeea boiiiiiiiiiiii";
    std::string message2 = "69";
    std::string message3 = "Yeeeeeaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa boiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii";

    std::vector<uint8_t> messageBuffer1(message1.size());
    memcpy(messageBuffer1.data(), message1.c_str(), message1.size());
    std::vector<uint8_t> messageBuffer2(message2.size());
    memcpy(messageBuffer2.data(), message2.c_str(), message2.size());
    std::vector<uint8_t> messageBuffer3(message3.size());
    memcpy(messageBuffer3.data(), message3.c_str(), message3.size());

    agent1.broadcastMessage(messageBuffer1);
    agent2.broadcastMessage(messageBuffer2);
    agent1.broadcastMessage(messageBuffer3);

    auto messageBufferRec1 = agent2.popMessage();
    std::string messageRec1((char *)messageBufferRec1.data(), messageBufferRec1.size());
    auto messageBufferRec2 = agent1.popMessage();
    std::string messageRec2((char *)messageBufferRec2.data(), messageBufferRec2.size());
    auto messageBufferRec3 = agent2.popMessage();
    std::string messageRec3((char *)messageBufferRec3.data(), messageBufferRec3.size());

    if (messageRec1.compare(message1) == 0)
        std::cout << "PASSED -- Broadcast test 3" << std::endl; // (" << message1 << ")" << std::endl;
    else std::cout << "FAILED -- Broadcast test 3 (" << messageRec1 << ")" << std::endl;

    if (messageRec2.compare(message2) == 0)
        std::cout << "PASSED -- Broadcast test 4" << std::endl; //(" << message2 << ")" << std::endl;
    else std::cout << "FAILED -- Broadcast test 4 (" << messageRec2 << ")" << std::endl;

    if (messageRec3.compare(message3) == 0)
        std::cout << "PASSED -- Broadcast test 5" << std::endl; // (" << message3 << ")" << std::endl;
    else std::cout << "FAILED -- Broadcast test 5 (" << messageRec3 << ")" << std::endl;

    return 0;
}
