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
// Authors: Radu Serban, Justin Madsen
// =============================================================================
//
// Main driver function for the HMMWV full model.
//
// The vehicle reference frame has Z up, X towards the front of the vehicle, and
// Y pointing to the left.
//
// =============================================================================

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/message.h>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <ctime>
#include <iostream>
#include <thread>
#include "ChronoMessages.pb.h"
#include "ServerVehicle.h"
#include "ChClient.h"

#include <memory>
#include <vector>

#include "chrono/core/ChFileutils.h"
#include "chrono/core/ChRealtimeStep.h"
#include "chrono/core/ChStream.h"
#include "chrono/physics/ChSystem.h"
#include "chrono/utils/ChUtilsInputOutput.h"

#include "ChBodyFollowerDriver.h"
#include "chrono_vehicle/ChConfigVehicle.h"
#include "chrono_vehicle/ChVehicleModelData.h"
#include "chrono_vehicle/driver/ChDataDriver.h"
#include "chrono_vehicle/driver/ChIrrGuiDriver.h"
#include "chrono_vehicle/terrain/RigidTerrain.h"
#include "chrono_vehicle/wheeled_vehicle/utils/ChWheeledVehicleIrrApp.h"
#include "models/vehicle/hmmwv/HMMWV.h"

#include "ChRaySensor.h"
using namespace chrono;
using namespace chrono::vehicle;
using namespace chrono::vehicle::hmmwv;
using boost::asio::ip::tcp;
using namespace irr;
// =============================================================================

// Initial vehicle location and orientation
ChVector<> initLoc(0, 0, 2.5);
ChQuaternion<> initRot(Q_from_AngZ(-1.57));
// ChQuaternion<> initRot(0.866025, 0, 0, 0.5);
// ChQuaternion<> initRot(0.7071068, 0, 0, 0.7071068);
// ChQuaternion<> initRot(0.25882, 0, 0, 0.965926);
// ChQuaternion<> initRot(0, 0, 0, 1);

enum DriverMode { DEFAULT, RECORD, PLAYBACK };
DriverMode driver_mode = DEFAULT;

// Visualization type for chassis & wheels (PRIMITIVES, MESH, or NONE)
VisualizationType vis_type = PRIMITIVES;

// Type of powertrain model (SHAFTS, SIMPLE)
PowertrainModelType powertrain_model = SHAFTS;

// Drive type (FWD, RWD, or AWD)
DrivelineType drive_type = AWD;

// Type of tire model (RIGID, PACEJKA, LUGRE, FIALA)
TireModelType tire_model = RIGID;

// Rigid terrain
RigidTerrain::Type terrain_model = RigidTerrain::FLAT;

double terrainHeight = 0;       // terrain height (FLAT terrain only)
double terrainLength = 1000.0;  // size in X direction
double terrainWidth = 1000.0;   // size in Y direction

// Point on chassis tracked by the camera
ChVector<> trackPoint(0.0, 0.0, 1.75);

// Simulation step sizes
double step_size = 0.001;
double tire_step_size = step_size;

// Simulation end time
double t_end = 1000;

// Time interval between two render frames
double render_step_size = 1.0 / 50;  // FPS = 50

// Output directories
const std::string out_dir = "../HMMWV";
const std::string pov_dir = out_dir + "/POVRAY";

// Debug logging
bool debug_output = false;
double debug_step_size = 1.0 / 1;  // FPS = 1

// POV-Ray output
bool povray_output = false;

// Vehicle message generating functions
ChronoMessages::VehicleMessage generateVehicleMessageFromWheeledVehicle(
    ChWheeledVehicle* vehicle, int connectionNumber);
void messageFromVector(ChronoMessages::VehicleMessage_MVector* message,
                       ChVector<> vector);
void messageFromQuaternion(ChronoMessages::VehicleMessage_MQuaternion* message,
                           ChQuaternion<> quaternion);
void generateChassisFromMessage(std::shared_ptr<ChBody> vehicle,
                                ChronoMessages::VehicleMessage& message);

// =============================================================================

int main(int argc, char* argv[]) {
  // --------------
  // Create systems
  // --------------

  // Create the HMMWV vehicle, set parameters, and initialize
  HMMWV_Full my_hmmwv;
  my_hmmwv.SetChassisFixed(false);
  my_hmmwv.SetChassisVis(vis_type);
  my_hmmwv.SetWheelVis(vis_type);
  my_hmmwv.SetInitPosition(ChCoordsys<>(initLoc, initRot));
  my_hmmwv.SetPowertrainType(powertrain_model);
  my_hmmwv.SetDriveType(drive_type);
  my_hmmwv.SetTireType(tire_model);
  my_hmmwv.SetTireStepSize(tire_step_size);
  my_hmmwv.Initialize();

  // create the lidar
  std::shared_ptr<ChRaySensor> lidar =
      std::make_shared<ChRaySensor>(my_hmmwv.GetChassis(), 30, true);
  lidar->Initialize(chrono::ChCoordsys<double>(
                        chrono::ChVector<double>({2.3, 0, 0}),  // offset x,y,z
                        chrono::ChQuaternion<double>(Q_from_NasaAngles(
                            {0, 0, 0}))),  // offset yaw,roll,pitch
                    1,
                    100, 0, 0, -1.5, 1.5, .2, 25);

  // Create the terrain
  RigidTerrain terrain(my_hmmwv.GetSystem());
  terrain.SetContactMaterial(0.9f, 0.01f, 2e7f, 0.3f);
  terrain.SetColor(ChColor(0.8f, 0.8f, 0.5f));
  switch (terrain_model) {
    case RigidTerrain::FLAT:
      terrain.SetTexture(vehicle::GetDataFile("terrain/textures/tile4.jpg"),
                         200, 200);
      terrain.Initialize(terrainHeight, terrainLength, terrainWidth);
      break;
    case RigidTerrain::HEIGHT_MAP:
      terrain.SetTexture(vehicle::GetDataFile("terrain/textures/grass.jpg"), 16,
                         16);
      terrain.Initialize(vehicle::GetDataFile("terrain/height_maps/test64.bmp"),
                         "test64", 128, 128, 0, 4);
      break;
    case RigidTerrain::MESH:
      terrain.SetTexture(vehicle::GetDataFile("terrain/textures/grass.jpg"),
                         100, 100);
      terrain.Initialize(vehicle::GetDataFile("terrain/meshes/test.obj"),
                         "test_mesh");
      break;
  }

  // Create other vehicles from network -- Work on this.
  // std::map<int, WheeledVehicle> otherVehicles; // Creating more wheeled
  // vehicles is less efficient.
  std::map<int, std::shared_ptr<ServerVehicle>> otherVehicles;
  std::map<int, std::shared_ptr<google::protobuf::Message>> worldVehicles;

  // Create the vehicle Irrlicht interface
  ChWheeledVehicleIrrApp app(&my_hmmwv.GetVehicle(), &my_hmmwv.GetPowertrain(),
                             L"HMMWV Demo");
  app.SetSkyBox();
  app.AddLightWithShadow(irr::core::vector3df(0.f, 0.f, 100.f),
                         irr::core::vector3df(0.f, 0.f, 0.f), 1000.0, 1.0,
                         1000.0, 90.0, 512,
                         irr::video::SColorf(1.f, 1.f, 1.f, 1.f), true, true);
  app.SetChaseCamera(trackPoint, 6.0, 0.5);
  app.SetTimestep(step_size);
  app.AssetBindAll();
  scene::IMeshSceneNode* node = app.GetSceneManager()->addMeshSceneNode(
      app.GetSceneManager()->getMesh("../data/madison_flat_mod.obj"), 0, -1,
      core::vector3df(150, 250, 0.01), core::vector3df(0, 0, 0),
      core::vector3df(1.0, 1.0, 1.0));
  app.AssetUpdateAll();

  // -----------------
  // Initialize output
  // -----------------

  if (ChFileutils::MakeDirectory(out_dir.c_str()) < 0) {
    std::cout << "Error creating directory " << out_dir << std::endl;
    return 1;
  }
  if (povray_output) {
    if (ChFileutils::MakeDirectory(pov_dir.c_str()) < 0) {
      std::cout << "Error creating directory " << pov_dir << std::endl;
      return 1;
    }
    terrain.ExportMeshPovray(out_dir);
  }

  std::string driver_file = out_dir + "/driver_inputs.txt";
  utils::CSV_writer driver_csv(" ");

  // ------------------------
  // Create the driver system
  // ------------------------
  std::cout << "making driver target" << std::endl;
  auto target = std::make_shared<ChBody>();
  std::cout << "done" << std::endl;

  // Create the interactive driver system
  ChBodyFollowerDriver driver(my_hmmwv.GetVehicle(), *target, 80, 3, 10, 3);
  // Set the time response for steering and throttle keyboard inputs.
  double steering_time = 1.0;  // time to go from 0 to +1 (or from 0 to -1)
  double throttle_time = 1.0;  // time to go from 0 to +1
  double braking_time = 0.3;   // time to go from 0 to +1
  //  driver.SetSteeringDelta(render_step_size / steering_time);
  //  driver.SetThrottleDelta(render_step_size / throttle_time);
  //  driver.SetBrakingDelta(render_step_size / braking_time);

  // If in playback mode, attach the data file to the driver system and
  // force it to playback the driver inputs.
  //  if (driver_mode == PLAYBACK) {
  //    driver.SetInputDataFile(driver_file);
  //    driver.SetInputMode(ChIrrGuiDriver::DATAFILE);
  //  }

  driver.Initialize();

  // ---------------
  // Simulation loop
  // ---------------

  // Number of simulation steps between miscellaneous events
  int render_steps = (int)std::ceil(render_step_size / step_size);
  int debug_steps = (int)std::ceil(debug_step_size / step_size);

  // Initialize simulation frame counter and simulation time
  ChRealtimeStepTimer realtime_timer;
  int step_number = 0;
  int render_frame = 0;
  double time = 0;

  // Setup client object and connect to network
  boost::asio::io_service ioService;
  ChClient client(&ioService);
  client.connectToServer(argv[1], "8082");
  client.asyncListen(worldVehicles);

  // Number of steps to wait before updating the server on the vehicle's
  // location
  int send_steps = render_steps;

  while (app.GetDevice()->run()) {
    time = my_hmmwv.GetSystem()->GetChTime();

    // End simulation
    if (time >= t_end) break;

    // Render scene and output POV-Ray data
    if (step_number % render_steps == 0) {
      app.BeginScene(true, true, irr::video::SColor(255, 140, 161, 192));
      app.DrawAll();
      app.EndScene();

      if (povray_output) {
        char filename[100];
        sprintf(filename, "%s/data_%03d.dat", pov_dir.c_str(),
                render_frame + 1);
        utils::WriteShapesPovray(my_hmmwv.GetSystem(), filename);
      }

      render_frame++;
    }

    // Debug logging
    if (debug_output && step_number % debug_steps == 0) {
      GetLog() << "\n\n============ System Information ============\n";
      GetLog() << "Time = " << time << "\n\n";
    }

    // Collect output data from modules (for inter-module communication)
    double throttle_input = driver.GetThrottle();
    double steering_input = driver.GetSteering();
    double braking_input = driver.GetBraking();

    // Driver output
    if (driver_mode == RECORD) {
      driver_csv << time << steering_input << throttle_input << braking_input
                 << std::endl;
    }

    // Update modules (process inputs from other modules)
    driver.Synchronize(time);
    terrain.Synchronize(time);
    my_hmmwv.Synchronize(time, steering_input, braking_input, throttle_input,
                         terrain);
    app.Synchronize("AUTONOMOUS", steering_input, throttle_input,
                    braking_input);

    // Send vehicle message
    if (step_number % send_steps == 0) {
      std::shared_ptr<google::protobuf::Message> message = std::make_shared<ChronoMessages::VehicleMessage>();
      message->MergeFrom(generateVehicleMessageFromWheeledVehicle(&my_hmmwv.GetVehicle(),
                client.connectionNumber()));
      client.sendMessage(message);

      for (std::pair<int, std::shared_ptr<google::protobuf::Message>> worldPair :
           worldVehicles) {
        if (otherVehicles.find(worldPair.first) ==
            otherVehicles.end()) {  // If the vehicle isn't found
          // Add a vehicle to the world
          auto newVehicle = std::make_shared<ServerVehicle>(
              my_hmmwv.GetVehicle().GetSystem());

          otherVehicles.insert(std::pair<int, std::shared_ptr<ServerVehicle>>(
              worldPair.first, newVehicle));
          if (client.connectionNumber() != 0 &&
              client.connectionNumber() == worldPair.first + 1) {
            std::cout << "Setting Target" << std::endl;

            driver.SetTarget(newVehicle->GetChassis());
          }

          app.AssetBindAll();
          app.AssetUpdateAll();
        }
        otherVehicles[worldPair.first]->update((ChronoMessages::VehicleMessage&)(*worldPair.second));
      }
      
      // Removing vehicles that have not received an update
      for(std::map<int, std::shared_ptr<ServerVehicle>>::iterator it = otherVehicles.begin(); it != otherVehicles.end();) {
        if(worldVehicles.find(it->first) == worldVehicles.end()){
          it = otherVehicles.erase(it);
          std::cout << "Vehicle removed" << std::endl;
        } else ++it;
      }
    }

    // Advance simulation for one timestep for all modules
    lidar->Update();
    driver.SetCurrentDistance(lidar->GetMinRange());
    double step = realtime_timer.SuggestSimulationStep(step_size);
    driver.Advance(step);
    terrain.Advance(step);
    my_hmmwv.Advance(step);
    app.Advance(step);

    // Increment frame number
    step_number++;
  }

  if (driver_mode == RECORD) {
    driver_csv.write_to_file(driver_file);
  }

  return 0;
}

ChronoMessages::VehicleMessage generateVehicleMessageFromWheeledVehicle(
    ChWheeledVehicle* vehicle, int connectionNumber) {
  ChronoMessages::VehicleMessage message;

  message.set_timestamp(time(0));
  message.set_vehicleid(connectionNumber);
  message.set_chtime(vehicle->GetChTime());
  message.set_speed(vehicle->GetVehicleSpeed());

  messageFromVector(message.mutable_chassiscom(), vehicle->GetChassisPos());
  messageFromVector(message.mutable_backleftwheelcom(),
                    vehicle->GetWheelPos(WheelID(1, LEFT)));
  messageFromVector(message.mutable_backrightwheelcom(),
                    vehicle->GetWheelPos(WheelID(1, RIGHT)));
  messageFromVector(message.mutable_frontleftwheelcom(),
                    vehicle->GetWheelPos(WheelID(0, LEFT)));
  messageFromVector(message.mutable_frontrightwheelcom(),
                    vehicle->GetWheelPos(WheelID(0, RIGHT)));

  messageFromQuaternion(message.mutable_chassisrot(), vehicle->GetChassisRot());
  messageFromQuaternion(message.mutable_backleftwheelrot(),
                        vehicle->GetWheelRot(WheelID(1, LEFT)));
  messageFromQuaternion(message.mutable_backrightwheelrot(),
                        vehicle->GetWheelRot(WheelID(1, RIGHT)));
  messageFromQuaternion(message.mutable_frontleftwheelrot(),
                        vehicle->GetWheelRot(WheelID(0, LEFT)));
  messageFromQuaternion(message.mutable_frontrightwheelrot(),
                        vehicle->GetWheelRot(WheelID(0, RIGHT)));

  return message;
}

void messageFromVector(ChronoMessages::VehicleMessage_MVector* message,
                       ChVector<> vector) {
  message->set_x(vector.x);
  message->set_y(vector.y);
  message->set_z(vector.z);
}

void messageFromQuaternion(ChronoMessages::VehicleMessage_MQuaternion* message,
                           ChQuaternion<> quaternion) {
  message->set_e0(quaternion.e0);
  message->set_e1(quaternion.e1);
  message->set_e2(quaternion.e2);
  message->set_e3(quaternion.e3);
}