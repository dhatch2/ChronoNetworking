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
//	Overrides the CalctargetLocation() function from ChSteeringController 
//	and uses the location of a tracked body passed in the constructor. 
//
// =============================================================================

#include "ChBodyFollowerSteeringController.h"

namespace chrono {
	namespace vehicle
	{
		ChBodyFollowerSteeringController::ChBodyFollowerSteeringController(ChBody &target) :target(&target)
		{
		}

		ChBodyFollowerSteeringController::~ChBodyFollowerSteeringController()
		{
		}

		void ChBodyFollowerSteeringController::CalcTargetLocation()
		{
			m_target = target->GetPos();
		}

		void ChBodyFollowerSteeringController::SetTarget(ChBody &target)
		{
			ChBodyFollowerSteeringController::target = &target;
		}



	}
}
