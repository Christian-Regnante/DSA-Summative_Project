# Task 1: Embedded Device Event Logger

## 1. Problem Overview
Industrial embedded monitoring devices operate under strict memory constraints while receiving continuous diagnostic alerts from sensors (e.g., temperature spikes, communication failures, voltage drops). 
This project implements an in-memory event logger using a **dynamically allocated Doubly Linked List** maintaining at most the **25 most recent events** in chronological order.

---

## 2. Data Structure Design & Architecture

### Why a Doubly Linked List?
1. **$O(1)$ Tail Appends:** New events are inserted at the tail in constant time.
2. **$O(1)$ Head Eviction:** When the 25-event limit is reached, the oldest event at the head is removed and freed in constant time without shifting array elements.
3. **Bidirectional Navigation:** The operator can move forward (`n`) and backward (`p`) through the event timeline seamlessly via `prev` and `next` pointers.

### Memory Layout
```
NULL <--- [Event 1: Oldest] <=======> [Event 2] <=======> [Event 3: Newest] ---> NULL
                 ^                        ^                      ^
               head                    current                  tail
```

---

## 3. Key Components & Implementation Details

* `EventNode`: Represents an event record storing `event_id`, `event_type`, `device_id`, `timestamp`, `severity`, and two pointers: `prev` and `next`.
* `EventLogger`: Controller storing pointers to `head`, `tail`, `current` (active viewing cursor), and `count` (0 to 25).
* **Head Eviction Guard:** If the operator is currently viewing the oldest event (`current == head`) when the 26th event arrives, `current` is safely advanced to `head->next` before freeing `head`, preventing dangling pointers.
* **Boundary Protections:** Navigation commands `n` and `p` safely check for `NULL` boundaries to prevent segmentation faults.

---

## 4. Time & Space Complexity

| Operation | Command | Time Complexity | Space Complexity |
| :--- | :---: | :---: | :---: |
| Initialization | Boot | $O(1)$ | $O(1)$ |
| Append Event | `a` | $O(1)$ | $O(1)$ per node |
| Evict Oldest | Auto on 26th | $O(1)$ | $O(1)$ freed |
| Navigate Next/Prev | `n` / `p` | $O(1)$ | $O(1)$ |
| Display Current | `c` | $O(1)$ | $O(1)$ |
| Clear All Events | `x` | $O(N)$ where $N \le 25$ | Releases all heap memory |
| Preload Samples (Demo) | `l` | $O(K)$ where $K = 24$ | $O(K)$ allocated |
| Save to File | `q` | $O(N)$ where $N \le 25$ | $O(1)$ auxiliary |

---

## 5. Compilation & Execution (Linux)

```bash
gcc -Wall -Wextra -std=c99 -pedantic main.c -o event_logger
./event_logger
```

---

## 6. Standardized Oral Defense (Q&A)

**Q1: Why choose a doubly linked list over a circular static array?**  
*Answer:* A doubly linked list allows dynamic memory allocation on demand, meaning memory is only occupied when events actually occur. It also allows arbitrary insertions/deletions if needed and makes bidirectional navigation straightforward with explicit pointer links.

**Q2: What happens if the operator is inspecting the oldest event when an eviction occurs?**  
*Answer:* The eviction logic specifically checks `if (logger->current == logger->head)`. If true, it advances `current` to `logger->head->next` *before* calling `free(old_head)`. This prevents `current` from becoming a dangling pointer referencing deallocated memory.

**Q3: How is memory leak prevention guaranteed upon program exit?**  
*Answer:* The quit command (`q`) and the fallback cleanup in `main` invoke `clear_all_events()`, which traverses the list from `head` to `tail`, saving `curr->next` into a temporary pointer and invoking `free(curr)` for every single node.
