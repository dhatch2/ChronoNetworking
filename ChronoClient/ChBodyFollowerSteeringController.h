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

#pragma once
#include "chrono_vehicle/utils/ChSteeringController.h"
#include "chrono/core/ChMathematics.h"

namespace chrono {
	namespace vehicle
	{
		class CH_VEHICLE_API ChBodyFollowerSteeringController :
			public ChSteeringController
		{
		public:
			ChBodyFollowerSteeringController(ChBody &target);
			~ChBodyFollowerSteeringController();
			void SetTarget(ChBody &target);

		private:
			ChBody *target;
			void CalcTargetLocation();
		};
	}
}



