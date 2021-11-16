//
// Created by xiaomaotou31 on 2021/11/16.
//

#ifndef MYTINYSERVER_BLOCKQUEUE_H
#define MYTINYSERVER_BLOCKQUEUE_H

#include <queue>
#include <iostream>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <cassert>
template <typename T>
class BlockQueue {
public:
    BlockQueue(int capacity_ = 20): capacity(capacity_){}
    T Pop();
    T Push(const T&);
    void clear();
    bool empty();
    typename T::size_type size();

private:
    std::queue<T> block_queue;
    std::condition_variable cv_producer;
    std::condition_variable cv_consumer;
    std::mutex queue_mutex;
    typename T::size_type capacity;
};

template<typename T>
T BlockQueue<T>::Pop() {
    std::unique_lock<std::mutex> lock(queue_mutex);
    cv_consumer.wait(lock, [this]{ return !block_queue.empty(); });
    assert(block_queue.empty());
    T value = block_queue.front();
    cv_producer.notify_one();
    return value;
}

template<typename T>
T BlockQueue<T>::Push(const T& new_v) {
    std::unique_lock<std::mutex> lock(queue_mutex);
    cv_producer.wait(lock, [this]{size() >= capacity;});
    block_queue.push(new_v);
    cv_consumer.notify_one();
}

template<typename T>
void BlockQueue<T>::clear() {
    std::unique_lock<std::mutex> lock(queue_mutex);
    block_queue.clear();
}

template<typename T>
bool BlockQueue<T>::empty() {
    std::unique_lock<std::mutex> lock(queue_mutex);
    return block_queue.empty();
}

template<typename T>
typename T::size_type BlockQueue<T>::size() {
    std::unique_lock<std::mutex> lock(queue_mutex);
    return block_queue.size();
}


#endif //MYTINYSERVER_BLOCKQUEUE_H
