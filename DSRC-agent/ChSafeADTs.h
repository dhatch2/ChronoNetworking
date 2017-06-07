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
#include <list>
#include <map>

template<class T> class ChSafeQueue {
public:
    void enqueue(const T& obj);
    const T dequeue();
    int size();

private:
    std::list<T> queue;
    std::mutex frontMutex;
    std::mutex endMutex;
};

template<class T, class K> class ChSafeMap {
public:
    T& operator[] (K key);
private:
    std::map<K, T> map;
    std::mutex mutex;
};

template<class T> void ChSafeQueue<T>::enqueue(const T& obj) {
    bool isLock = false;
    frontMutex.lock();
    if (queue.size() == 0) {
        endMutex.lock();
        isLock = true;
    }
    queue.push_back(obj);
    if (isLock) endMutex.unlock();
    frontMutex.unlock();
}

template<class T> const T ChSafeQueue<T>::dequeue() {
    T obj = queue.front();
    bool isLock = false;
    endMutex.lock();
    if (queue.size() == 1){
        frontMutex.lock();
        isLock = true;
    }
    queue.pop_front();
    if (isLock) frontMutex.unlock();
    endMutex.unlock();
    return obj;
}

template<class T> int ChSafeQueue<T>::size() {
    return queue.size();
}

#endif
