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
//	Helper functions used to convert between ChVehicle and VehicleMessages.
//
// =============================================================================

#ifndef MESSAGECONVERSIONS_H
#define MESSAGECONVERSIONS_H

#include "ChronoMessages.pb.h"
#include "chrono/physics/ChSystem.h"
#include "chrono_models/vehicle/hmmwv/HMMWV.h"

using namespace chrono;
using namespace chrono::vehicle;

ChronoMessages::VehicleMessage generateVehicleMessageFromWheeledVehicle(ChWheeledVehicle* vehicle, int connectionNumber, int idNumber);
void messageFromVector(ChronoMessages::MVector* message,
                       ChVector<> vector);
void messageFromQuaternion(ChronoMessages::MQuaternion* message,
                           ChQuaternion<> quaternion);

#endif
