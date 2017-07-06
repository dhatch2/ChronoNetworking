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

#include "MessageConversions.h"

using namespace chrono;
using namespace chrono::vehicle;

ChronoMessages::VehicleMessage generateVehicleMessageFromWheeledVehicle(
    ChWheeledVehicle* vehicle, int connectionNumber, int idNumber) {
    ChronoMessages::VehicleMessage message;

    message.set_timestamp(time(0));
    message.set_idnumber(idNumber);
    message.set_connectionnumber(connectionNumber);
    message.set_chtime(vehicle->GetChTime());
    message.set_speed(vehicle->GetVehicleSpeed());

    messageFromVector(message.mutable_chassiscom(), vehicle->GetChassis()->GetPos());
    messageFromVector(message.mutable_backleftwheelcom(),
                      vehicle->GetWheelPos(WheelID(1, LEFT)));
    messageFromVector(message.mutable_backrightwheelcom(),
                      vehicle->GetWheelPos(WheelID(1, RIGHT)));
    messageFromVector(message.mutable_frontleftwheelcom(),
                      vehicle->GetWheelPos(WheelID(0, LEFT)));
    messageFromVector(message.mutable_frontrightwheelcom(),
                      vehicle->GetWheelPos(WheelID(0, RIGHT)));

    messageFromQuaternion(message.mutable_chassisrot(), vehicle->GetChassis()->GetRot());
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

void messageFromVector(ChronoMessages::MVector* message,
                       ChVector<> vector) {
    message->set_x(vector.x());
    message->set_y(vector.y());
    message->set_z(vector.z());
}

void messageFromQuaternion(ChronoMessages::MQuaternion* message,
                           ChQuaternion<> quaternion) {
    message->set_e0(quaternion.e0());
    message->set_e1(quaternion.e1());
    message->set_e2(quaternion.e2());
    message->set_e3(quaternion.e3());
}
