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

#include "ChRayShape.h"

#include "chrono/physics/ChSystem.h"
#include "chrono/assets/ChTexture.h"
#include "chrono/assets/ChColorAsset.h"




ChRayShape::ChRayShape(std::shared_ptr<chrono::ChBody> parent, bool visualize){
	this->parent = parent;	//save reference to the ChSystem
	this->visualize = visualize;

	//these are set to -1 for now since they have not been used ***Possible Problem***
	this->contactLen = -1;
	this->contactFiducial = -1;
	this->contactRetro = -1;
}

/// \brief Constructor
/// \param body Link the ray is attached to
//public: BulletRayShape(CollisionPtr _collision);

/// \brief Destructor
ChRayShape::~ChRayShape(){

}

/// update the ray collision
/// this checks the ray and sets the appropriate ray length and hit object name
void ChRayShape::Update(bool updateCollision){

	this->globalStartPos = this->parent->GetPos() + this->parent->GetRot().Rotate(relativeStartPos);
	this->globalEndPos = this->parent->GetPos() + this->parent->GetRot().Rotate(relativeEndPos);

	if(updateCollision){
		// see if the ray collides with anything
		this->parent->GetSystem()->GetCollisionSystem()->RayHit(this->globalStartPos, this->globalEndPos, this->rayCollision);
		//std::cout<<"Ray collided: "<<rayCollision.hit<<std::endl;
		if(rayCollision.hit){
			//if the ray collides, set the length accordingly
			this->SetLength((rayCollision.abs_hitPoint-this->globalStartPos).Length());
			//std::cout<<"Ray Collided: "<<rayCollision.hit<<"\tRange: "<<this->GetLength()<<"\tHit: "<<rayCollision.hitModel->GetPhysicsItem()->GetNameString()<<std::endl;
		}
	}
	//std::cout<<"Vert Pos: "<<this->GetPos().y;
	if(visualize){
		rayEnd->SetPos(globalEndPos);
		rayStart->SetPos(globalStartPos);
	}
}

///This finds the intersection of the ray and the colliding object
void ChRayShape::GetIntersection(double &_dist, std::string &_entity){
	//see if the ray collides with anything
	this->parent->GetSystem()->GetCollisionSystem()->RayHit(this->globalStartPos, this->globalEndPos, this->rayCollision);
	//std::cout<<"Ray collided: "<<rayCollision.hit<<std::endl;
	if(rayCollision.hit){
		//if the ray collides, send back the distance and the entity name with which it collided
		_dist = (rayCollision.abs_hitPoint-this->globalStartPos).Length();
		_entity = rayCollision.hitModel->GetPhysicsItem()->GetNameString();

		//std::cout<<"Ray Collided: "<<rayCollision.hit<<"\tRange: "<<_dist<<"Hit: "<<_entity<<std::endl;
	}
}

/// set the start and end point of the ray
///
/// \param posStart Start position, relative the body
/// \param posEnd End position, relative to the body
void ChRayShape::SetPoints(const chrono::ChVector<double> &_posStart,
		const chrono::ChVector<double> &_posEnd){
	//set the global start and end points
	this->relativeStartPos = _posStart;
	this->relativeEndPos = _posEnd;
	this->globalStartPos = this->parent->GetPos() + this->parent->GetRot().Rotate(_posStart);
	this->globalEndPos = this->parent->GetPos() + this->parent->GetRot().Rotate(_posEnd);

	if(visualize){
		//insert a small sphere at the end of each ray for debugging purposes
		rayEnd = std::make_shared<chrono::ChBodyEasySphere>(.02, 3000, false, true);
		rayEnd->SetPos(chrono::ChVector<>(this->globalEndPos));
		rayEnd->SetBodyFixed(true);

		this->parent->GetSystem()->Add(rayEnd);

		auto color2 = std::make_shared<chrono::ChColorAsset>();
		color2->SetColor(chrono::ChColor(0.1f, 1.0f, .1f));
		rayEnd->AddAsset(color2);

		rayStart = std::make_shared<chrono::ChBodyEasySphere>(.02, 3000, false, true);
		rayStart->SetPos(chrono::ChVector<>(this->globalStartPos));
		rayStart->SetBodyFixed(true);

		this->parent->GetSystem()->Add(rayStart);

		auto color3 = std::make_shared<chrono::ChColorAsset>();
		color3->SetColor(chrono::ChColor(0.1f, 0.1f, 1.0f));
		rayStart->AddAsset(color3);

		//this->SetPos({this->globalEndPos});
	}

}

/// set the length of the ray
void ChRayShape::SetLength(double _len){
	//set the length to the contact
	this->contactLen = _len;

	chrono::ChVector<double> dir = this->relativeEndPos - this->relativeStartPos;
	dir.Normalize();

	// this was in gazebo RayShape or BulletRayShape but causes the end position to be incorrect the next time around
	this->relativeEndPos = dir*_len + this->relativeStartPos;
	//this->SetPoints(this->globalStartPos, this->globalEndPos);
	//this->SetPos({this->globalEndPos});


}

/// returns the length of the ray
double ChRayShape::GetLength() const{
	//return the length to contact
	return this->contactLen;

}














