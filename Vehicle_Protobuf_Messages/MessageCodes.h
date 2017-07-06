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

#define DSRC_MESSAGE 6

#define MESSAGE_PACKET 7

#define CONNECTION_REQUEST 8
#define CONNECTION_ACCEPT 9
#define CONNECTION_DECLINE 10

#define VEHICLE_MESSAGE_TYPE "ChronoMessages.VehicleMessage"
#define VEHICLE_MESSAGE_SIZE 361
#define DSRC_MESSAGE_TYPE "ChronoMessages.DSRCMessage"
#define MESSAGE_PACKET_TYPE "ChronoMessages.MessagePacket"

#define CONNECTION_NUMBER_FIELD "connectionNumber"
#define ID_NUMBER_FIELD "idNumber"

#endif
