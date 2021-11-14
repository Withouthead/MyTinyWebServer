//
// Created by xiaomaotou31 on 2021/11/14.
//

#include "HeapTimer.h"

void HeapTimer::AddNde(int id, int timeout, const TimeoutCallBack &call_back) {
    if(node_pos.count(id) != 0)
    {
        auto index = node_pos.find(id)->second;
        heap[index].expires = Clock::now() + MS(timeout);
        heap[index].call_back = call_back;
        Heapify(index);
    }
    else
    {
        auto index = heap.size();
        TimerNode new_node;
        new_node.call_back = call_back;
        new_node.id = id;
        new_node.expires = Clock::now() + MS(timeout);
        heap.emplace_back(std::move(new_node));
        Heapify(index);
    }


}

void HeapTimer::HeapifyDown(size_t index) {
    auto heap_size = heap.size();
    assert(index < heap_size);
    size_t child = index * 2 + 1;
    while(child < heap_size)
    {
        if(child + 1 < heap_size && heap[child + 1] < heap[child])
            child += 1;
        if(heap[child] < heap[index])
        {
            SwapNode(index, child);
            index = child;
            child = index * 2 + 1;
        }
        else
            break;
    }
}

void HeapTimer::HeapifyUp(size_t index) {
    auto heap_size = heap.size();
    assert(index < heap_size);
    size_t father = (index - 1) / 2;
    while(father >= 0)
    {
        if(heap[index] < heap[father])
        {
            SwapNode(index, father);
            index = father;
            father = (index - 1) / 2;
        }
        else
            break;
    }
}

void HeapTimer::SwapNode(size_t i, size_t j) {
    assert(i >= 0 && i < heap.size());
    assert(j >= 0 && j < heap.size());
    std::swap(heap[i], heap[j]);
    node_pos[heap[i].id] = j;
    node_pos[heap[j].id] = i;

}

void HeapTimer::Heapify(size_t index) {
    HeapifyUp(index);
    HeapifyDown(index);

}

void HeapTimer::DelNode(int index) {
    size_t heap_size = heap.size();
    assert(index < heap_size && index >= 0);
    size_t del_index = heap_size - 1;
    if(del_index > index)
    {
        SwapNode(index, del_index);
        Heapify(index);
    }
    node_pos.erase(heap.back().id);
    heap.pop_back();
}

void HeapTimer::Tick() {
    if(heap.empty())
        return;
    while(!heap.empty())
    {
        TimerNode node = heap.front();
        if(std::chrono::duration_cast<MS>(node.expires - Clock::now()).count() > 0)
            break;
        node.call_back();
        DelNode(0);
    }
}

int HeapTimer::GetTopTick() {
    Tick();
    size_t res = -1;
    if(!heap.empty())
        res = std::chrono::duration_cast<MS>(heap.front().expires - Clock::now()).count();
    return res;
}

void HeapTimer::Clear() {
    heap.clear();
    node_pos.clear();
}

void HeapTimer::Update(int id, int timeout) {
    assert(!heap.empty() && node_pos.count(id));
    size_t index = node_pos.find(id)->second;
    heap[index].expires = Clock::now() + MS(timeout);
}

