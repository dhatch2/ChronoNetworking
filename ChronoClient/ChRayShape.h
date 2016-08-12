//
// PROJECT CHRONO - http://projectchrono.org
//
// Copyright (c) 2013 Project Chrono
// All rights reserved.
//
//Author: Asher Elmquist
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file at the top level of the distribution
// and at http://projectchrono.org/license-chrono.txt.
//

//************************************************************************
//NOTICE: this file is modified from it's original source. Original source
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

#include "chrono/physics/ChSystem.h"
#include "chrono/core/ChCoordsys.h"
#include "chrono/core/ChVector.h"
#include "chrono/physics/ChBodyEasy.h"
#include "chrono/physics/ChBody.h"


#include <vector>
#include <math.h>
#include <stdio.h>

class ChRayShape
{
public: ChRayShape(std::shared_ptr<chrono::ChBody> parent, bool visualize);

/// \brief Constructor
/// \param body Link the ray is attached to
//public: BulletRayShape(CollisionPtr _collision);

/// \brief Destructor
public: ~ChRayShape();

/// \brief Update the ray collision
public: void Update(bool updateCollision);

/// \brief Get the nearest intersection
public: void GetIntersection(double &_dist, std::string &_entity);

/// \brief Set the ray based on starting and ending points relative to
///        the body
/// \param posStart Start position, relative the body
/// \param posEnd End position, relative to the body
public: void SetPoints(const chrono::ChVector<double> &_posStart,
		const chrono::ChVector<double> &_posEnd);

/// \brief Pointer to the Bullet physics engine
private: std::shared_ptr<chrono::ChBody> parent;
private: bool visualize;

/// \brief Set the length of the ray.
/// \param[in] _len Length of the array.
public: void SetLength(double _len);

public: double GetLength() const;


private: double contactLen;
/// \brief Retro reflectance value
private: double contactRetro;
/// \brief Fiducial ID value.
private: int contactFiducial;

/// \brief Start position of the ray in global cs
private: chrono::ChVector<double> globalStartPos;

/// \brief End position of the ray in global cs
private: chrono::ChVector<double> globalEndPos;

private: chrono::ChVector<double> relativeStartPos;

/// \brief End position of the ray in global cs
private: chrono::ChVector<double> relativeEndPos;
private: chrono::collision::ChCollisionSystem::ChRayhitResult rayCollision;

private: std::shared_ptr<chrono::ChBodyEasySphere> rayEnd;
private: std::shared_ptr<chrono::ChBodyEasySphere> rayStart;

};









