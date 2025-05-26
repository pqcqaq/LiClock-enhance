#ifndef CUSTOM_LIST_H
#define CUSTOM_LIST_H

#include <stddef.h> // for size_t

template <typename T>
class List {
private:
    struct Node {
        T data;
        Node* prev;
        Node* next;
        Node(const T& val) : data(val), prev(nullptr), next(nullptr) {}
    };

    Node* head;
    Node* tail;
    size_t count;

public:
    List() : head(nullptr), tail(nullptr), count(0) {}

    ~List() {
        clear();
    }

    void push_back(const T& value) {
        Node* node = new Node(value);
        if (!tail) {
            head = tail = node;
        } else {
            tail->next = node;
            node->prev = tail;
            tail = node;
        }
        ++count;
    }

    void pop_back() {
        if (!tail) return;

        Node* toDelete = tail;
        tail = tail->prev;
        if (tail)
            tail->next = nullptr;
        else
            head = nullptr; // list is now empty

        delete toDelete;
        --count;
    }

    T& back() {
        // 你可以根据项目需要加 assert 或错误检查
        return tail->data;
    }

    void clear() {
        Node* current = head;
        while (current) {
            Node* next = current->next;
            delete current;
            current = next;
        }
        head = tail = nullptr;
        count = 0;
    }

    size_t size() const {
        return count;
    }

    bool empty() const {
        return count == 0;
    }

    // 简易迭代器支持
    class iterator {
        Node* ptr;
    public:
        iterator(Node* p) : ptr(p) {}
        iterator() : ptr(nullptr) {} // ✅ 默认构造函数

        iterator& operator++() { ptr = ptr->next; return *this; }
        iterator operator++(int) { iterator tmp = *this; ++(*this); return tmp; }

        T& operator*() { return ptr->data; }
        T* operator->() { return &ptr->data; }

        bool operator==(const iterator& other) const { return ptr == other.ptr; }
        bool operator!=(const iterator& other) const { return ptr != other.ptr; }
    };

    iterator begin() { return iterator(head); }
    iterator end() { return iterator(nullptr); }

    // 禁用拷贝（可按需启用）
    List(const List&) = delete;
    List& operator=(const List&) = delete;
};

#endif // CUSTOM_LIST_H
