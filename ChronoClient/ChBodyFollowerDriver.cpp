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
// Authors: Devin Lafford
// =============================================================================
//
//	This is a driver model that follows a ChBody using
//	ChAdaptiveSpeedController and ChBodyFollowerSteeringController. The SetGains
//	functions in the constructor body can be used to change the behaviors of the
//	speed PID and the steeering PID.
//
// =============================================================================

#include "ChBodyFollowerDriver.h"
#include "chrono_vehicle/ChVehicle.h"
#include "math.h"

ChBodyFollowerDriver::ChBodyFollowerDriver(ChVehicle &vehicle,
	ChBody &follow_target,
	double target_speed,
	double target_following_time,
	double target_min_distance,
	double current_distance) : ChDriver(vehicle),
	m_target_speed(target_speed),
	m_target_following_time(target_following_time),
	m_target_min_distance(target_min_distance),
	m_current_distance(current_distance),
	m_throttle_threshold(0.2),
	self(vehicle),
	target(&follow_target),
	m_steeringPID(follow_target)
{
	//default behavior; can be overridden later with m_xxxxxPID.SetGains(Kp,Ki,Kd)
	m_steeringPID.SetGains(0.1, 0.0, 0.02);
	m_speedPID.SetGains(1.0, 0.2, 0.2);

	//init everything to zero
	SetSteering(0);
	m_speedPID.Reset(m_vehicle);
	m_steeringPID.Reset(m_vehicle);
}

ChBodyFollowerDriver::~ChBodyFollowerDriver()
{
}

void ChBodyFollowerDriver::Reset()
{
	SetSteering(0);
	m_speedPID.Reset(m_vehicle);
	m_steeringPID.Reset(m_vehicle);
}

void ChBodyFollowerDriver::Advance(double step)
{
	ChVector<> target_pos = target->GetPos();
	ChVector<> self_pos = self.GetDriverPos();

	// Set the steering value based on the output from the steering controller.
	double out_steering = m_steeringPID.Advance(self, step);
	ChClampValue(out_steering, -1.0, 1.0);
	m_steering = out_steering;

	double m_current_distance = sqrt((target_pos.x() - self_pos.x())*(target_pos.x() - self_pos.x()) + (target_pos.y() - self_pos.y())*(target_pos.y() - self_pos.y()) + (target_pos.z() - self_pos.z())*(target_pos.z() - self_pos.z()));
	m_steeringPID.SetLookAheadDistance(m_current_distance);

	double out_speed = m_speedPID.Advance(m_vehicle, m_target_speed, m_target_following_time, m_target_min_distance, m_current_distance, step);
	ChClampValue(out_speed, -1.0, 1.0);

	if (out_speed > 0) {
		// Vehicle moving too slow
		m_braking = 0;
		m_throttle = out_speed;
	}
	else if (m_throttle > m_throttle_threshold) {
		// Vehicle moving too fast: reduce throttle
		m_braking = 0;
		m_throttle = 1 + out_speed;
	}
	else {
		// Vehicle moving too fast: apply brakes
		m_braking = -out_speed;
		m_throttle = 0;
	}
}
