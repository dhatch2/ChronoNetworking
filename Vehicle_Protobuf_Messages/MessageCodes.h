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
//	Macros used to define network communication.
//
// =============================================================================

#ifndef MESSAGECODES_H
#define MESSAGECODES_H

#define NULL_MESSAGE 0
#define DISCONNECT_MESSAGE 1
#define VEHICLE_MESSAGE 2
#define VEHICLE_MESSAGE_END 3
#define VEHICLE_ID 4
#define HEARTBEAT 5

#define VEHICLE_MESSAGE_TYPE "ChronoMessages.VehicleMessage"
#define VEHICLE_MESSAGE_SIZE 361

#endif
