#include <chrono>
#include <iostream>
#include <thread>
#include <numeric>
#include <iterator>
#include <vector>
#include <cmath>
#include <curl/curl.h>
#include <climits>
#include <future>
#include <string>


#include <regex>
#include <set>
#include <mutex>
#include <queue>
#include <condition_variable>


// Queue from TD5
template <class E>
class SafeUnboundedQueueCV {
    std::queue<E> elements;
    std::mutex lock;
    std::condition_variable not_empty;
public:
    SafeUnboundedQueueCV<E>(){}
    void push(const E& element);
    E pop ();
    bool is_empty() const {return this->elements.empty();}
};

template <class E>
void SafeUnboundedQueueCV<E>::push(const E& element) {
    std::unique_lock<std::mutex> guard(this->lock);
    bool was_empty = this->elements.size() == 0; //inspired by the demo code
    this->elements.push(element);
    if (was_empty){
        this->not_empty.notify_all();  //could be notify_one here but probably
    }
}

template <class E>
E SafeUnboundedQueueCV<E>::pop() {
    std::unique_lock<std::mutex> guard(this->lock);
    while (this->is_empty()){
        this->not_empty.wait(guard);
    }
    E first_element = elements.front();
    this->elements.pop();
    return first_element;
}

