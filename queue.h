//
// Created by oskarosw3 on 5/2/26.
//

#ifndef PARALLEL_CRAWLER_QUEUE_H
#define PARALLEL_CRAWLER_QUEUE_H



#include <mutex>
#include <queue>
#include <condition_variable>

template <class E>
class SafeUnboundedQueueCV {
    std::queue<E> elements;
    std::mutex lock;
    std::condition_variable not_empty;
public:
    SafeUnboundedQueueCV() {}
    void push(const E& element);
    E pop();
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
        this->not_empty.wait(guard);
    }
    E first_element = elements.front();
    this->elements.pop();
    return first_element;
}

#endif //PARALLEL_CRAWLER_QUEUE_H