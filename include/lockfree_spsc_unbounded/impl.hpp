#ifndef LOCKFREE_SPSC_UNBOUNDED_IMPL
#define LOCKFREE_SPSC_UNBOUNDED_IMPL

#include "defs.hpp"

template <typename T>
using queue = tsfqueue::__impl::lockfree_spsc_unbounded<T>;

template <typename T> void queue<T>::push(T value) {
     node* new_stub  = new node;
        new_stub->next.store(nullptr,std::memory_order_relaxed);
        tail->data = std::move(value);
        sz.fetch_add(1,std::memory_order_relaxed);
        tail->next.store(new_stub,std::memory_order_release);
        tail = new_stub;
}

template <typename T> bool queue<T>::try_pop(T &value) {
node* next = head->next.load(std::memory_order_acquire);
        static_assert(std::is_move_assignable<T>::value,
                  "T must be move assignable for try_pop to work\n");
        static_assert(std::is_nothrow_move_assignable<T>::value,"T's move assignment  must not throw exceptions!");
        if(next==nullptr){
          return false;
        }      
        else {
          value = std::move(head->data);
          node* old = head;
          head = next;
          delete old;
          sz.fetch_sub(1,std::memory_order_relaxed);
          return true;
      }
}

template <typename T> void queue<T>::wait_and_pop(T &value) {
    while((head->next.load(std::memory_order_acquire))==nullptr) continue;
        node* next = head->next.load(std::memory_order_acquire);
        sz.fetch_sub(1,std::memory_order_relaxed);
        value = std::move(head->data);
        node* old = head;
        head = next;
        delete old;
}

template <typename T> bool queue<T>::peek(T &value) {
    if(empty()) return false;
      else {
        value = head->data;
        return true;
      }
}

template <typename T> bool queue<T>::empty(void) {
    return head->next.load(std::memory_order_acquire)==nullptr;
}

template <typename T> size_t size(void){
    return sz.load(std::memory_order_relaxed);
}

#endif

// 1. Add static asserts
// 2. Add emplace_back using perfect forwarding and variadic templates (you
// can use this in push then)
// 3. Add size() function
// 4. Any more suggestions ??