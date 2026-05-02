//
// Created by oskarosw3 on 5/2/26.
//

#ifndef PARALLEL_CRAWLER_COARSE_SET_H
#define PARALLEL_CRAWLER_COARSE_SET_H


#include <string>
#include <mutex>
#include <vector>
#include <future>
#include <chrono>
#include <cstdint>

class Node {
public:
    std::string item;
    uint64_t key;
    Node *next;
    Node() {}
    Node(const std::string& s) {
        this->item = s;
        this->key = std::hash<std::string>{}(s);
        this->next = NULL;
    }
    Node(uint64_t k) {
        this->item = "";
        this->key = k;
        this->next = NULL;
    }
};

void DeleteNodeChain(Node* start);

class CoarseSetList {
protected:
    Node* head;
    static const uint64_t LOWEST_KEY = 0;
    static const uint64_t LARGEST_KEY = std::numeric_limits<uint64_t>::max();
    Node* search(const std::string& val);
    std::recursive_mutex lock;
public:
    CoarseSetList();
    ~CoarseSetList();
    bool add(const std::string& val);
    bool remove(const std::string& val);
    bool contains(const std::string& val);
    void print() const;
    size_t size() const;
};


#endif //PARALLEL_CRAWLER_COARSE_SET_H