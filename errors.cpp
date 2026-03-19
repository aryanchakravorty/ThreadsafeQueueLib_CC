/*
Errors, Logical Bugs, and Optimizations for blocking_mpmc_unbounded:

1. [SYNTAX ERROR] missing closing parenthesis for `cond.wait` in `impl.hpp`.
   - File: `include/blocking_mpmc_unbounded/impl.hpp` (line 52-54)
   - Problem: `cond.wait(head_lock, [this]){ return !empty(); }` lacks the closing `);`.
   - Fix: `cond.wait(head_lock, [this]{ return !empty(); });`

2. [COMPILATION ERROR] `size()` implementation is not a member function.
   - File: `include/blocking_mpmc_unbounded/impl.hpp` (line 70)
   - Problem: The definition `size_t size()` is global and lacks the `queue<T>::` qualifier. It also tries to access private members.
   - Fix: Change to `template <typename T> size_t queue<T>::size() { ... }`

3. [COMPILATION ERROR] `size()` used before definition in `try_get`.
   - File: `include/blocking_mpmc_unbounded/impl.hpp` (line 62)
   - Problem: `size()` is called in `try_get` before its implementation appears.
   - Fix: Move the implementation of `size()` above `try_get` or any other caller.

4. [COMPILATION ERROR] `static_assert` syntax error.
   - File: `include/blocking_mpmc_unbounded/defs.hpp` (line 120-125)
   - Problem: `std::is_copy_constructible_v(T)` uses parentheses instead of angle brackets.
   - Fix: Use `std::is_copy_constructible_v<T>`, etc.

5. [LOGICAL BUG / OPTIMIZATION] Missing `std::move` in `try_pop(T& value)`.
   - File: `include/blocking_mpmc_unbounded/impl.hpp` (line 99)
   - Problem: `value = *(removed_node->data);` performs a copy even if T is movable.
   - Fix: Use `value = std::move(*(removed_node->data));`

6. [LOGICAL BUG] Inconsistent/Inefficient node extraction in `wait_for_and_get`.
   - File: `include/blocking_mpmc_unbounded/impl.hpp` (line 186-191)
   - Problem: `std::unique_ptr<node> new_head = std::move(head->next); ... head = std::move(new_head);` is unnecessarily verbose and slightly less efficient than the logic in `wait_and_get`.
   - Fix: Use the same logic as `wait_and_get`: `std::unique_ptr<node> old_head = std::move(head); head = std::move(old_head->next); return old_head;`

7. [INCOMPLETE IMPLEMENTATION] `wait_for_pop` is declared but not implemented.
   - File: `include/blocking_mpmc_unbounded/defs.hpp` (line 134-135)
   - Problem: Two overloads of `wait_for_pop` are declared but not found in `impl.hpp`. (Note: `wait_for_and_pop` is implemented, which seems to be the intended name, but the header has both).
   - Fix: Either remove the duplicate declarations or provide aliases.

8. [OPTIMIZATION] Unnecessary mutex for size tracking.
   - File: `include/blocking_mpmc_unbounded/defs.hpp` (line 34-35)
   - Problem: Using a separate `std::mutex size_mutex` for `size_q` adds overhead and complexity.
   - Fix: Use `std::atomic<size_t> size_q;` and remove `size_mutex`.

9. [OPTIMIZATION] `std::shared_ptr` usage in `push`.
   - File: `include/blocking_mpmc_unbounded/impl.hpp` (line 13)
   - Problem: `std::make_shared<T>(std::move(value))` is good, but `emplace_back` is even better for avoiding copies.
   - Fix: (Already partially implemented in `emplace_back`, but ensure `push` uses it or follows similar logic).

10. [SYNTAX ERROR] Double semicolon.
    - File: `include/blocking_mpmc_unbounded/impl.hpp` (line 21)
    - Problem: `tail = tail->next.get();;`
    - Fix: Remove the extra semicolon.

11. [TYPE MISMATCH] `return 0;` and `return 1;` for `bool`.
    - File: `include/blocking_mpmc_unbounded/impl.hpp` (line 97-100, 153, etc.)
    - Problem: Returning integers for boolean functions.
    - Fix: Use `return false;` and `return true;`.

12. [TYPO] "exclusive excess".
    - File: `include/blocking_mpmc_unbounded/impl.hpp` (line 132, 148, 163)
    - Problem: Comment says "excess" instead of "access".
    - Fix: Change "excess" to "access".

13. [REDUNDANCY] Redundant includes in `queue.hpp`.
    - File: `include/blocking_mpmc_unbounded/queue.hpp`
    - Problem: Includes both `defs.hpp` and `impl.hpp`, where `impl.hpp` already includes `defs.hpp`.
    - Fix: Only include `impl.hpp`.

14. [POTENTIAL LOGICAL BUG] `clear()` doesn't notify.
    - File: `include/blocking_mpmc_unbounded/impl.hpp` (line 216)
    - Problem: While not strictly a bug, clearing a queue that threads are waiting on might leave them blocked forever if no new pushes occur.
    - Fix: Call `cond.notify_all();` at the end of `clear()`.

15. [DESIGN FLAW] Static asserts in the wrong place.
    - File: `include/blocking_mpmc_unbounded/defs.hpp` (line 120)
    - Problem: Static asserts are inside the class body but at the end. Usually they should be near the top or outside.
    - Fix: Keep them, but fix the angle bracket syntax error.
*/
