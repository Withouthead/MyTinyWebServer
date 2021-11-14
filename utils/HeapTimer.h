//
// Created by xiaomaotou31 on 2021/11/14.
//

#ifndef MYTINYSERVER_HEAPTIMER_H
#define MYTINYSERVER_HEAPTIMER_H

#include <chrono>
#include <algorithm>
#include <vector>
#include <functional>
#include <unordered_map>
#include <cassert>

using TimeStamp = std::chrono::steady_clock::time_point;
using Clock = std::chrono::steady_clock;
using TimeoutCallBack = std::function<void()>;
using MS = std::chrono::milliseconds;
struct TimerNode
{
    int id;
    TimeStamp expires;
    TimeoutCallBack call_back;
    bool operator < (const TimerNode& t) const
    {
        return expires < t.expires;
    }
};
class HeapTimer {
    void AddNde(int id, int timeout, const TimeoutCallBack& call_back);
    void DelNode(int index);
    void Tick();
    int GetTopTick();
    void Clear();
    void Update(int id, int timeout);
private:
    void HeapifyDown(size_t index);
    void HeapifyUp(size_t index);
    void Heapify(size_t index);
    void SwapNode(size_t i, size_t j);
    std::vector<TimerNode> heap;
    std::unordered_map<int, std::vector<TimerNode>::size_type> node_pos;

};


#endif //MYTINYSERVER_HEAPTIMER_H
