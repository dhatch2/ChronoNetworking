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

#ifndef CHSAFEADTS_H
#define CHSAFEADTS_H

#include <mutex>
#include <queue>
#include <map>

template<class T> class ChSafeQueue {
public:
    void enqueue(const T& obj);
    const T& dequeue();

private:
    std::queue<T> queue;
    std::mutex mutex;
};

template<class T, class K> class ChSafeMap {
public:
    T& operator[] (K key);
private:
    std::map<K, T> map;
    std::mutex mutex;
};

#endif
