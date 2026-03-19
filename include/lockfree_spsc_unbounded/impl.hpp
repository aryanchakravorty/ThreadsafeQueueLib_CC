#ifndef LOCKFREE_SPSC_UNBOUNDED_IMPL
#define LOCKFREE_SPSC_UNBOUNDED_IMPL

#include "defs.hpp"

template <typename T>
using queue = tsfqueue::__impl::lockfree_spsc_unbounded<T>;

template <typename T>
void queue<T>::push(T value)
{
    node *new_stub = new node;
    new_stub->next.store(nullptr, std::memory_order_relaxed);
    tail->data = std::move(value);
    capacity.fetch_add(1, std::memory_order_relaxed);
    tail->next.store(new_stub, std::memory_order_release);
    tail = new_stub;
}

template <typename T>
bool queue<T>::try_pop(T &value)
{
    static_assert(std::is_move_assignable_v<T>,
                  "T must be move assignable");
    static_assert(std::is_nothrow_destructible_v<T>, "T must be nothrow destructible");
    node *next = head->next.load(std::memory_order_acquire);
    if (next == nullptr)
    {
        return false;
    }
    else
    {
        value = std::move(head->data);
        node *old = head;
        head = next;
        delete old;
        capacity.fetch_sub(1, std::memory_order_relaxed);
        return true;
    }
}

template <typename T>
void queue<T>::wait_and_pop(T &value)
{
    static_assert(std::is_move_assignable_v<T>,
                  "T must be move assignable");
    static_assert(std::is_nothrow_destructible_v<T>, "T must be nothrow destructible");

    while ((head->next.load(std::memory_order_acquire)) == nullptr)
        continue;
    node *next = head->next.load(std::memory_order_acquire);
    capacity.fetch_sub(1, std::memory_order_relaxed);
    value = std::move(head->data);
    node *old = head;
    head = next;
    delete old;
}

template <typename T>
bool queue<T>::peek(T &value)
{
    static_assert(std::is_copy_assignable_v<T>, "T must be copy assignable");
    if (empty()) return false;
    else
    {
        value = head->data;
        return true;
    }
}

template <typename T>
bool queue<T>::empty(void)
{
    return head->next.load(std::memory_order_acquire) == nullptr;
}

template <typename T>
size_t queue<T>::size(void)
{
    return capacity.load(std::memory_order_relaxed);
}

#endif

// 1. Add static asserts
// 2. Add emplace_back using perfect forwarding and variadic templates (you
// can use this in push then)
// 3. Add size() function
// 4. Any more suggestions ??