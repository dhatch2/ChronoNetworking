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
//	Defines thread-safe ADTs used in the Chrono CAVE project.
//
// =============================================================================

#include "ChSafeADTs.h"

template<class T> void ChSafeQueue<T>::enqueue(const T& obj) {
    mutex.lock();
    queue.push(obj);
    mutex.unlock();
}

template<class T> const T& ChSafeQueue<T>::dequeue() {
    T obj = queue.front();
    mutex.lock();
    queue.pop();
    mutex.unlock();
    return obj;
}
