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




/*
This demo shows the coarse-grained thread-safe set based on a linked list.
To observe: no elements lost in parallel insertion (the size + number of collisions is equal to the number of insertions),
runtimes goes slightly up as number of threads increaes, not down (natural for the coarse-grain approach)
*/


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

class CoarseSetList {
protected:
    Node* head;
    static const uint64_t LOWEST_KEY = 0;
    static const uint64_t LARGEST_KEY = std::numeric_limits<uint64_t>::max();
    // returns the pointer to the last node with key < hash(val)
    Node* search(const std::string& val);
    // recusive to allow function taking lock to call another function taking the same lock
    std::recursive_mutex lock;
public:
    CoarseSetList() {
        this->head = new Node(CoarseSetList::LOWEST_KEY);
        this->head->next = new Node(CoarseSetList::LARGEST_KEY);
    }
    ~CoarseSetList();
    bool add(const std::string& val);
    bool remove(const std::string& val);
    bool contains(const std::string& val);
    void print() const; // for testing
    size_t size() const; // for testing
};

// No lock here, so not thread-safe
unsigned long CoarseSetList::size() const {
    size_t result = 0;
    Node* cur = this->head->next;
    while (cur->next != NULL) {
        ++result;
        cur = cur->next;
    }
    return result;
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

//------------------------------------------------------

// returns the number of collisions
int inserter(CoarseSetList& set, int count) {
    int collisions = 0;
    for (int i = 0; i < count; ++i) {
        bool flag = set.add(std::to_string(rand()));
        if (!flag) {
            ++collisions;
        }
    }
    return collisions;
}

// returns a pair of (runtime, collisions)
std::pair<int, int> benchmark_multi_threads(CoarseSetList& set, int count, int num_threads) {
    std::vector<std::future<int>> collisions;
    int collisions_count = 0;
    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < num_threads - 1; ++i) {
        collisions.push_back(std::async(std::launch::async, &inserter, std::ref(set), count / num_threads));
    }
    collisions_count = inserter(set, count - (num_threads - 1) * (count / num_threads));
    for (int i = 0; i < num_threads - 1; ++i) {
        collisions_count += collisions[i].get();
    }
    auto finish = std::chrono::steady_clock::now();
    int elapsed = std::chrono::duration_cast<std::chrono::microseconds>(finish - start).count();
    return std::make_pair(elapsed, collisions_count);
}

//------------------------------------------------------

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Provide two arguments: num_thread, num_insertions" << std::endl;
        return 0;
    }

    int num_threads = std::stoi(argv[1]);
    int num_insertions = std::stoi(argv[2]);

    CoarseSetList CL;
    auto res = benchmark_multi_threads(CL, num_insertions, num_threads);
    std::cout << "Time for coare-grained version is " << res.first << " microseconds" << std::endl;
    std::cout << "Number of elements is " << CL.size() << std::endl;
    std::cout << "Number of collisions is " << res.second << std::endl;
}
