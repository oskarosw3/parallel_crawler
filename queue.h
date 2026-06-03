//
// Created by oskarosw3 on 5/2/26.
//

#ifndef PARALLEL_CRAWLER_QUEUE_H
#define PARALLEL_CRAWLER_QUEUE_H



#include <mutex>
#include <queue>
#include <condition_variable>

#include "SetList.h"

template <class E>
class SafeUnboundedQueueCV {
    std::queue<E> elements;
    std::mutex lock;
    std::condition_variable not_empty;

public:
    std::atomic<int> threads_waiting = 0;
    bool finished = false;
    int total_threads;
    SafeUnboundedQueueCV() {}
    void push(const E& element);
    E pop();
    E check_and_pop();
    void wake_up();

    bool is_empty() const { return this->elements.empty(); }
};

template <class E>
void SafeUnboundedQueueCV<E>::push(const E& element) {
    std::unique_lock<std::mutex> guard(this->lock);
    bool was_empty = this->elements.size() == 0;
    this->elements.push(element);
    if (was_empty) {
        this->not_empty.notify_all();
    }
}

template <class E>
E SafeUnboundedQueueCV<E>::pop() {
    std::unique_lock<std::mutex> guard(this->lock);
    while (this->is_empty()) {
        threads_waiting.fetch_add(1);
        this->not_empty.wait(guard);
        threads_waiting.fetch_sub(1);
    }
    E first_element = elements.front();
    this->elements.pop();
    return first_element;
}

template <class E>
E SafeUnboundedQueueCV<E>::check_and_pop() {
    std::unique_lock<std::mutex> guard(this->lock);
    if (threads_waiting.load() == total_threads - 1 ) {
        if (is_empty()) {
            finished = true; //signals to other threads that program terminates
            wake_up();
            return E();

        }
    }
    threads_waiting.fetch_add(1);
    while (this->is_empty()) {


        // dispose of lock while it waits
        this->not_empty.wait(guard);
        if (finished){ return E();}

    }
    threads_waiting.fetch_sub(1);
    E first_element = elements.front();
    this->elements.pop();
    return first_element;
}

template <class E>
void SafeUnboundedQueueCV<E>::wake_up() {
    not_empty.notify_all();
}

#endif //PARALLEL_CRAWLER_QUEUE_H