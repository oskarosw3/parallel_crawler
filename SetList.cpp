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

#include "SetList.h"


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

unsigned long SetList::size() const {
    unsigned long result = 0;
    Node* cur = this->head->next;
    while (cur->next != NULL) {
        ++result;
        cur = cur->next;
    }
    return result;
}

SetList::SetList() {
    this->head = new Node(SetList::LOWEST_KEY);
    this->head->next = new Node(SetList::LARGEST_KEY);
}

SetList::~SetList() {
    DeleteNodeChain(this->head);
}

Node* SetList::search(const std::string& val) const {
    Node *pred, *curr;
    unsigned long key = std::hash<std::string>{}(val);
    pred = head;
    pred->lock.lock();
    curr = pred->next;
    curr->lock.lock();
    while (curr->key < key) {
        pred->lock.unlock();
        pred = curr;
        curr = curr->next;
        curr->lock.lock();
    }
    return pred; 
}

bool SetList::add(const std::string& val) {
    Node* pred = this->search(val);
    Node* curr = pred->next;
    bool exists = (curr->key == std::hash<std::string>{}(val));
    if (!exists) {
        Node* node = new Node(val);
        node->next = curr;
        pred->next = node;
    }
    pred->lock.unlock();
    curr->lock.unlock();
    return !exists;
}

bool SetList::add_and_update_distance(const std::string& val, int distance, std::set<std::string> links, std::string& parent) {
    Node* pred = this->search(val);
    Node* curr = pred->next;
    bool exists = (curr->key == std::hash<std::string>{}(val));

    if (!exists) {
        Node* node = new Node(val);
        node->next = curr;
        node->distance = distance;
        node->parent = parent;
        pred->next = node;
    }
    else {
        if (distance < curr->distance) {
            curr->distance = distance;
            curr->parent = parent;
        }
        links = curr->site_links;
    }
    pred->lock.unlock();
    curr->lock.unlock();
    return exists;
}

bool SetList::add_links(const std::string& val, std::set<std::string> links) {
    Node* pred = this->search(val);
    Node* curr = pred->next;

    curr -> site_links = links;
    curr -> has_links = true;

    pred->lock.unlock();
    curr->lock.unlock();
    return 1;
}

bool SetList::remove(const std::string& val) {
    Node* pred = this->search(val);
    Node* curr = pred->next;
    bool exists = (curr->key == std::hash<std::string>{}(val));
    curr->lock.unlock();
    if (exists) {
        pred->next = curr->next;
        delete curr;
    }
    pred->lock.unlock();
    return exists;
}

bool SetList::contains(const std::string& val) const {
    Node* pred = this->search(val);
    Node* curr = pred->next;
    bool exists = (curr->key == std::hash<std::string>{}(val));
    pred->lock.unlock();
    curr->lock.unlock();
    return exists;
}

bool SetList::contains_and_distance(const std::string& val, int& distance) {
    Node* pred = this->search(val);
    Node* curr = pred->next;
    bool exists = (curr->key == std::hash<std::string>{}(val));
    distance = curr->distance;
    pred->lock.unlock();
    curr->lock.unlock();
    return exists;
}


void SetList::print() const {
    Node* cur = this->head->next;
    while (cur->next != NULL) {
        std::cout << cur->item << " ";
        cur = cur->next;
    }
    std::cout << std::endl;
}

