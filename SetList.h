#ifndef PARALLEL_CRAWLER_SET_LIST_H
#define PARALLEL_CRAWLER_SET_LIST_H

#include <string>
#include <mutex>
#include <climits>
#include <functional>

class Node {
public:
    std::mutex lock;
    std::string item;
    int distance; //how far away from the main link it is
    unsigned long key;

    std::set<std::string> site_links;
    bool has_links = false;
    std::string parent;

    Node * next;
    Node() {}
    Node(const std::string& s) {
        this->item = s;
        this->key = std::hash<std::string>{}(s);
        this->next = NULL;
    }
    Node(unsigned long k) {
        this->item = "";
        this->key = k;
        this->next = NULL;
    }

};

void DeleteNodeChain(Node* start);

class SetList {
protected:
    Node* head;
    static const unsigned long LOWEST_KEY = 0;
    static const unsigned long LARGEST_KEY = ULONG_MAX;

    // returns the pointer to the last node with key < hash(val)
    // with keeping this and the next nodes locked
    Node* search(const std::string& val) const;

public:
    SetList();
    ~SetList();
    //those functions for auto&, they are not thread-safe but they are only used in a non-concurrent part
    Node* begin () {return head;}
    Node* end_aux(Node* node) {
        if (node->next == NULL) {
            return node;
        }
        return end_aux(node->next);
    }
    Node* end() { return end_aux(head);}

    bool add(const std::string& val);
    bool add_and_update_distance(const std::string& val, int distance, std::set<std::string> links, std::string& parent);
    bool add_links(const std::string& val, std::set<std::string> links);
    bool remove(const std::string& val);
    bool contains(const std::string& val) const;
    bool contains_and_distance(const std::string& val, int& distance);

    template <typename F>
    void transform(F f);

    void print() const; // for testing
    unsigned long size() const; // for testing
};

#endif //PARALLEL_CRAWLER_SET_LIST_H