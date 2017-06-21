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

class PredicateException : public std::exception {
    virtual const char* what() const throw() {
        return "Predicate failed. Queue closed.";
    }
};

template<class T> class ChSafeQueue {
public:
    ChSafeQueue();
    ChSafeQueue(std::function<bool()> pred);
    void notifyPredicate();
    void enqueue(const T& obj);
    T& dequeue();
    int size();
    void dumpThreads();

private:
    std::function<bool()> predicate;
    std::queue<T> queue;
    std::mutex mutex;
    std::condition_variable var;
    bool dump;
};

template<class T> ChSafeQueue<T>::ChSafeQueue() : predicate([]{ return true; }) {
    dump = false;
}

template<class T> ChSafeQueue<T>::ChSafeQueue(std::function<bool()> pred) : predicate(pred) {
    dump = false;
}

template<class T> void ChSafeQueue<T>::notifyPredicate() {
    var.notify_one();
}

template<class T> void ChSafeQueue<T>::enqueue(const T& obj) {
    if (!predicate()) throw PredicateException();
    T* newObj = new T(obj);
    std::unique_lock<std::mutex> lock(mutex);
    queue.push(*newObj);
    lock.unlock();
    var.notify_one();
}

template<class T> T& ChSafeQueue<T>::dequeue() {
    std::unique_lock<std::mutex> lock(mutex);
    var.wait(lock, [&]{ return !queue.empty() || !predicate() || dump; });
    if (!predicate() || dump) {
        lock.unlock();
        var.notify_one();
        throw PredicateException();
    }
    T* obj = new T(queue.front());
    queue.pop();
    return *obj;
}

template<class T> int ChSafeQueue<T>::size() {
    return queue.size();
}

template<class T> void ChSafeQueue<T>::dumpThreads() {
    dump = true;
    var.notify_one();
}

#endif
