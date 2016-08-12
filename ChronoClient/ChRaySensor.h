//
// PROJECT CHRONO - http://projectchrono.org
//
// Copyright (c) 2013 Project Chrono
// All rights reserved.
//
// Author: Asher Elmquist
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file at the top level of the distribution
// and at http://projectchrono.org/license-chrono.txt.
//

//************************************************************************
// NOTICE: this file is modified from it's original source. Original source
//		  is based on work done by Open Source Robotics Foundation
//************************************************************************

/*
 * Copyright (C) 2012-2016 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

/**
 * This is based on ODEMultiRayShape.hh and MultiRayShape.hh
 *
 */
#include "ChRayShape.h"
#include "ChSensor.h"

#include "chrono/core/ChCoordsys.h"
#include "chrono/core/ChVector.h"
#include "chrono/physics/ChBody.h"
#include "chrono/physics/ChSystem.h"

#include <math.h>
#include <stdio.h>
#include <vector>

class ChRaySensor : public ChSensor {
  // \brief Constructor.
  /// \param[in] _parent Parent collision shape.
 public:
  /// Constructor
  ChRaySensor(std::shared_ptr<chrono::ChBody> parent, double updateRate,
              bool visualize);

  /// Destructor
  ~ChRaySensor();

  /// Initialize the ChRaySensor
  void Initialize(chrono::ChCoordsys<double> offsetPose, int horzSamples,
                  int vertSamples, double horzMinAngle, double horzMaxAngle,
                  double vertMinAngle, double vertMaxAngle, double minRange,
                  double maxRange);
  /***TEMPORART PARENT OBJECT***/
  //,chrono::ChCoordsys<double> parentPose);

  /// Update the rays
  void UpdateRays();

  double GetRange(unsigned int _index);
  double GetMinRange();
  std::vector<double> Ranges();

  void Update();

  /// Add a ray to the collision
 private:
  void AddRay(const chrono::ChVector<double> &start,
              const chrono::ChVector<double> &end);

  // private: std::shared_ptr<chrono::ChBody> parent;
  // private: bool visualize;

 private:
  std::vector<std::shared_ptr<ChRayShape>> rays;

 private:
  chrono::ChCoordsys<double> offsetPose;

 private:
  double minRange = -1;

 private:
  double maxRange = -1;
};
