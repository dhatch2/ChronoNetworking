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

#include "ChRaySensor.h"

#include <stdio.h>
#include <cmath>
#include <iostream>
#include <vector>

#include "chrono/assets/ChColorAsset.h"
#include "chrono/assets/ChTexture.h"
#include "chrono/physics/ChBodyEasy.h"
#include "chrono/physics/ChLinkLock.h"
#include "chrono/physics/ChSystem.h"

using namespace chrono;

/// Constructor for a ChRaySensor
ChRaySensor::ChRaySensor(std::shared_ptr<ChBody> parent, double updateRate,
                         bool visualize)
    : ChSensor(parent, updateRate, visualize) {
  // save a reference to the ChSystem
  //	this->parent = parent;
  //	this->visualize = visualize;

  //	this->SetDensity(.1);
  //	this->SetMass(.1);
  //	this->SetInertiaXX(ChVector<>(.1,.1,.1));
  //	auto vshape = std::make_shared<ChBoxShape>();
  //	vshape->GetBoxGeometry().SetLengths(ChVector<>(.05, .05, .05));
  //	this->AddAsset(vshape);
}
ChRaySensor::~ChRaySensor() { this->rays.clear(); }

/// Initialize the ChRaySensor
/// pass in: vertMinAngle, vertMaxAngle, vertSamples, horzMinAngle,
/// horzMaxAngle,horzSamples,minRange,maxRange
void ChRaySensor::Initialize(chrono::ChCoordsys<double> offsetPose,
                             int horzSamples, int vertSamples,
                             double horzMinAngle, double horzMaxAngle,
                             double vertMinAngle, double vertMaxAngle,
                             double minRange, double maxRange) {
  /***TEMPORARY PARENT OBJECT***/
  //,chrono::ChCoordsys<double> parentPose){

  this->minRange = minRange;
  this->maxRange = maxRange;

  if (vertSamples < 1) vertSamples = 1;
  if (horzSamples < 1) horzSamples = 1;

  chrono::ChVector<double> start, end, axis;
  double yawAngle, pitchAngle;
  chrono::ChQuaternion<double> ray;
  double yDiff = horzMaxAngle - horzMinAngle;
  double pDiff = vertMaxAngle - vertMinAngle;

  for (unsigned int j = 0; j < (unsigned int)vertSamples; ++j) {
    for (unsigned int i = 0; i < (unsigned int)horzSamples; ++i) {
      yawAngle =
          (horzSamples == 1) ? 0 : i * yDiff / (horzSamples - 1) + horzMinAngle;

      pitchAngle =
          (vertSamples == 1) ? 0 : j * pDiff / (vertSamples - 1) + vertMinAngle;

      // conduct the rotation
      // conduct the rotation

      // NOT USING THE COLLISION PARENT - MAY BE WRONG ROTATION BECAUSE OF THIS
      /****NEED TO ADD IN COLLISION PARENT TO ROTATE BASED ON THAT******/
      // Yaw, Roll, Pitch according to ChQuaternion.h
      ray.Q_from_NasaAngles(
          chrono::ChVector<double>(yawAngle, 0.0, -pitchAngle));

      /********************THIS HAS A BUG*****************************/
      /********************SOMETHING WITH ROTATIONS - PARTICULARLY
       * OFFSET*****************/
      axis = (offsetPose.rot * ray).Rotate(chrono::ChVector<double>(1.0, 0, 0));

      start = (axis * minRange) + offsetPose.pos;
      end = (axis * maxRange) + offsetPose.pos;

      this->AddRay(start, end);
      // std::cout<<"Added ray: "<<i<<", "<<j<<" at start: "<<start.x<<",
      // "<<start.y<<", "<<start.z<<" , end: "<<end.x<<", "<<end.y<<",
      // "<<end.z<< std::endl;
    }
  }
}

/// \brief Physics engine specific method for updating the rays.
void ChRaySensor::UpdateRays() {}

/// \brief Add a ray to the collision.
/// \param[in] _start Start of the ray.
/// \param[in] _end End of the ray.
void ChRaySensor::AddRay(const chrono::ChVector<double> &_start,
                         const chrono::ChVector<double> &_end) {
  std::shared_ptr<ChRayShape> ray =
      std::make_shared<ChRayShape>(this->parent, visualize);

  // this->parent->GetSystem()->Add(ray);

  // auto link = std::make_shared<chrono::ChLinkLockLock>();
  // std::shared_ptr<chrono::ChBody> this_ptr(this);
  //	auto tempBox = std::make_shared<ChBodyEasyBox>(.1, .1, .1,  // x, y, z
  // dimensions
  //			3000,       // density
  //			true,      // enable contact geometry
  //			true        // enable visualization geometry
  //	);
  // tempBox->SetPos(this->GetPos());

  // tempBox->SetBodyFixed(true);

  // mphysicalSystem->Add(tempBox);
  // link->Initialize(std::make_shared<ChRaySensor>(*this),ray,true,this->GetCoord(),ray->GetCoord());
  // mphysicalSystem->AddLink(link);
  // this->AddMarker(ray);
  // this->AddAsset(ray->Get);
  ray->SetPoints(_start, _end);
  this->rays.push_back(ray);
}

double ChRaySensor::GetRange(unsigned int _index) {
  if (_index >= this->rays.size()) {
    // std::cout<<"Index out of bounds
    return -1;
  }
  // add min range because we measured from min range
  return this->minRange + this->rays[_index]->GetLength();
}

double ChRaySensor::GetMinRange() {
  double shortest = this->rays.at(0)->GetLength();
  for (int i = 1; i < this->rays.size(); i++) {
    shortest = (rays.at(i)->GetLength() < shortest) ? rays.at(i)->GetLength() : shortest;
  }

  // add min range because we measured from min range
  return this->minRange + shortest;
}

std::vector<double> ChRaySensor::Ranges() {
  std::vector<double> ranges = std::vector<double>();
  for (int i = 0; i < this->rays.size(); i++) {
    ranges.push_back(this->minRange + rays[i]->GetLength());
  }
  return ranges;
}

void ChRaySensor::Update() {
  double fullRange = this->maxRange - this->minRange;

  bool updateCollisions = false;
  if (this->parent->GetChTime() >= timeLastUpdated + 1.0 / updateRate) {
    updateCollisions = true;
    timeLastUpdated = this->parent->GetChTime();
    for (unsigned int i = 0; i < this->rays.size(); i++) {
      this->rays[i]->SetLength(fullRange);
    }
    // std::cout<<"UpdateRays: "<<this->parent->GetChTime()<<std::endl;
  }
  std::vector<std::shared_ptr<ChRayShape>>::iterator iter;
  for (iter = this->rays.begin(); iter != this->rays.end(); ++iter) {
    (*iter)->Update(updateCollisions);
  }
}
