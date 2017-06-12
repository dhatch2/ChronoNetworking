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
#include <condition_variable>

template<class T> class ChSafeQueue {
public:
    void enqueue(const T& obj);
    T& dequeue();
    int size();

private:
    std::queue<T> queue;
    std::mutex mutex;
    std::condition_variable var;
};

template<class T> void ChSafeQueue<T>::enqueue(const T& obj) {
    T* newObj = new T(obj);
    std::unique_lock<std::mutex> lock(mutex);
    queue.push(*newObj);
    lock.unlock();
    var.notify_one();
}

template<class T> T& ChSafeQueue<T>::dequeue() {
    std::unique_lock<std::mutex> lock(mutex);
    var.wait(lock, [&]{ return !queue.empty(); });
    T* obj = new T(queue.front());
    queue.pop();
    return *obj;
}

template<class T> int ChSafeQueue<T>::size() {
    return queue.size();
}

#endif
