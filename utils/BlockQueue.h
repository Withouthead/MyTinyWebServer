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
    BlockQueue(int capacity_ = 20): capacity(capacity_), is_close(false){}
    bool Pop(T&);
    bool Push(const T&);
    void clear();
    bool empty();
    void Close();
    void flush();
    typename T::size_type size();

private:
    std::queue<T> block_queue;
    std::condition_variable cv_producer;
    std::condition_variable cv_consumer;
    std::mutex queue_mutex;
    bool is_close;
    typename T::size_type capacity;
};

template<typename T>
bool BlockQueue<T>::Pop(T& item) {
    std::unique_lock<std::mutex> lock(queue_mutex);
    cv_consumer.wait(lock, [this]{ return !block_queue.empty() || is_close; });
    if(is_close)
        return false;
    assert(block_queue.empty() == false);
    T value = block_queue.front();
    item = value;
    block_queue.pop();
    cv_producer.notify_one();
    return true;
}

template<typename T>
bool BlockQueue<T>::Push(const T& new_v) {
    std::unique_lock<std::mutex> lock(queue_mutex);
    cv_producer.wait(lock, [this]{ return block_queue.size() <= capacity;});
    if(is_close)
        return false;
    block_queue.push(new_v);
    cv_consumer.notify_one();
    return true;
}

template<typename T>
void BlockQueue<T>::clear() {
    std::unique_lock<std::mutex> lock(queue_mutex);
    std::queue<T> empty;
    std::swap( block_queue, empty );
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

template<typename T>
void BlockQueue<T>::flush() {
    cv_consumer.notify_one();
}

template<typename T>
void BlockQueue<T>::Close() {
    std::unique_lock<std::mutex> lock(queue_mutex);
    std::queue<T> empty;
    std::swap( block_queue, empty );
    is_close = true;
    lock.unlock();
    cv_consumer.notify_all();
    cv_producer.notify_all();

}


#endif //MYTINYSERVER_BLOCKQUEUE_H
