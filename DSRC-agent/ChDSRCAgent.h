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
//	Agent for vehicle-to-vehicle communication via DSRC.
//
// =============================================================================

#ifndef CHDSRCAGENT_H
#define CHDSRCAGENT_H

#include <iostream>
#include <utility>
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
#include <boost/asio.hpp>

using namespace chrono;
using namespace chrono::vehicle;
using namespace chrono::vehicle::hmmwv;

class ChDSRCAgent {
public:
    ChDSRCAgent(ChWheeledVehicle* veh);
    bool canReach(int vehicle);
    int vehicleNumber();
    void broadcastMessage(std::vector<uint8_t> buffer);
    std::vector<uint8_t> popMessage();
private:
    ChWheeledVehicle* vehicle;
    int m_vehicleNumber;
    std::queue<std::shared_ptr<boost::asio::streambuf>> incomingMessages;

    bool canReach(ChDSRCAgent *vehicle);
};

#endif
