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

#pragma once
#include "chrono_vehicle/ChDriver.h"
#include "chrono_vehicle/ChVehicle.h"
#include "chrono_vehicle/utils/ChAdaptiveSpeedController.h"
#include "ChBodyFollowerSteeringController.h"

using namespace chrono;
using namespace chrono::vehicle;
//using namespace chrono::vehicle::hmmwv;

namespace chrono {
	namespace vehicle {
		class CH_VEHICLE_API ChBodyFollowerDriver :
			public ChDriver
		{
		public:
			ChVehicle &self;	//the vehicle being controlled
			ChBody *target;	//the vehicle being tracked

			ChBodyFollowerDriver(ChVehicle &vehicle, ChBody &target, double target_speed, double target_following_time, double target_min_distance, double current_distance);
			~ChBodyFollowerDriver();

			void SetDesiredSpeed(double val) { m_target_speed = val; }

			void SetDesiredFollowingTime(double val) { m_target_following_time = val; }

			void SetDesiredFollowingMinDistance(double val) { m_target_min_distance = val; }

			void SetCurrentDistance(double val) { m_current_distance = val; }

			void SetThreshholdThrottle(double val) { m_throttle_threshold = val; }

			void SetTarget(ChBody &target)//possible to change tracked vehicle on the fly
			{
				ChBodyFollowerDriver::target = &target;
				m_steeringPID.SetTarget(target);
			}

			ChAdaptiveSpeedController& GetSpeedController() { return m_speedPID; }

			ChBodyFollowerSteeringController &GetSteeringController() { return m_steeringPID; }

			void Reset();

			virtual void Advance(double step) override;

			ChBodyFollowerSteeringController m_steeringPID; ///< steering controller
			ChAdaptiveSpeedController m_speedPID;    ///< speed controller

		private:
			double m_target_speed;                   ///< desired vehicle speed
			double m_target_following_time;          ///< desired min following time gap
			double m_target_min_distance;            ///< desired min distance to the vehicle in front
			double m_current_distance;               ///< current distance to the vehicle in front
			double m_throttle_threshold;             ///< throttle value below which brakes are applied
		};

	}//end namespace vehicle

}//end namespace chrono
