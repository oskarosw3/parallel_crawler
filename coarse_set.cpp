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

#include "coarse_set.h"



/*
This demo shows the coarse-grained thread-safe set based on a linked list.
To observe: no elements lost in parallel insertion (the size + number of collisions is equal to the number of insertions),
runtimes goes slightly up as number of threads increaes, not down (natural for the coarse-grain approach)
*/


void DeleteNodeChain(Node* start) {
    Node* prev = start;
    Node* cur = prev->next;
    while (cur != NULL) {
        delete prev;
        prev = cur;
        cur = cur->next;
    }
    delete prev;
}

CoarseSetList::CoarseSetList() {
    this->head = new Node(CoarseSetList::LOWEST_KEY);
    this->head->next = new Node(CoarseSetList::LARGEST_KEY);
}

CoarseSetList::~CoarseSetList() {
    std::lock_guard<std::recursive_mutex> lk(this->lock);
    DeleteNodeChain(this->head);
}

Node* CoarseSetList::search(const std::string& val) {
    std::lock_guard<std::recursive_mutex> lk(this->lock);
    Node *pred, *curr;
    uint64_t key = std::hash<std::string>{}(val);
    pred = head;
    curr = pred->next;
    while (curr->key < key) {
        pred = curr;
        curr = curr->next;
    }
    return pred;
}

bool CoarseSetList::add(const std::string& val) {
    std::lock_guard<std::recursive_mutex> lk(this->lock);
    Node* pred = this->search(val);
    Node* curr = pred->next;
    bool exists = (curr->key == std::hash<std::string>{}(val));
    if (!exists) {
        Node* node = new Node(val);
        node->next = curr;
        pred->next = node;
    }
    return !exists;
}

bool CoarseSetList::remove(const std::string& val) {
    std::lock_guard<std::recursive_mutex> lk(this->lock);
    Node* pred = this->search(val);
    Node* curr = pred->next;
    bool exists = (curr->key == std::hash<std::string>{}(val));
    if (exists) {
        pred->next = curr->next;
        delete curr;
    }
    return exists;
}

bool CoarseSetList::contains(const std::string& val) {
    std::lock_guard<std::recursive_mutex> lk(this->lock);
    Node* pred = this->search(val);
    Node* curr = pred->next;
    bool exists = (curr->key == std::hash<std::string>{}(val));
    return exists;
}

unsigned long CoarseSetList::size() const {
    size_t result = 0;
    Node* cur = this->head->next;
    while (cur->next != NULL) {
        ++result;
        cur = cur->next;
    }
    return result;
}

