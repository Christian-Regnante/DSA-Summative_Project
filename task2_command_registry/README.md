# Task 2: Network Device Command Registry

## 1. Problem Overview
Embedded network appliances (routers, switches, industrial gateways) require instant verification of diagnostic and configuration commands typed by operators. Unapproved commands must be caught, recorded in an audit log with timestamps, and typos should receive helpful suggestions without compromising security.

This project implements a command registry backed by a **dynamically allocated Binary Search Tree (BST)** with a strict 50-command capacity limit.

---

## 2. Data Structure Design & Architecture

### Why a Binary Search Tree (BST)?
1. **$O(\log N)$ Average Search Time:** Finding whether a command exists requires binary comparison (`strcmp`), discarding half the search space at each level.
2. **Natural Alphabetical Ordering:** An **In-Order Traversal** (`Left -> Node -> Right`) naturally outputs all commands in sorted lexicographical order without needing an extra sorting step.
3. **Dynamic Memory Allocation:** Nodes are allocated on demand up to the 50-command capacity ceiling.

```
                    [ RESET_INTERFACE ]
                         /         \
                       /             \
            [ DELETE_ROUTE ]        [ SET_IP ]
               /        \               \
        [ ADD_ROUTE ] [ ENABLE_FW ]   [ SHOW_STATUS ]
```

---

## 3. Key Implementation Details

* **File Ingestion & Duplicate Suppression:** Reads commands from `commands.txt`. If `strcmp == 0`, the duplicate is rejected, ensuring the tree only stores unique entries.
* **Exact Command Search:** Recursively traverses left if `strcmp < 0` and right if `strcmp > 0`.
* **Typo Suggestion (Levenshtein Distance):** If a command is not found, the tree is scanned to compute edit distance. Any approved command within $\le 2$ character edits (insertions, deletions, substitutions) is suggested (`Did you mean: ...?`).
* **Audit Logging:** Every unapproved command is recorded in `audit.log` formatted as:  
  `YYYY-MM-DD HH:MM:SS | UNKNOWN | <command>`
* **Post-Order Deallocation:** Memory is cleanly freed via post-order traversal (`Left -> Right -> Root`) on `x` and `q`.

---

## 4. Time & Space Complexity

| Operation | Command | Average Time Complexity | Worst-Case Time | Space Complexity |
| :--- | :---: | :---: | :---: | :---: |
| BST Insertion | File Load | $O(\log N)$ | $O(N)$ | $O(1)$ per node |
| Exact Verification | `v` | $O(\log N)$ | $O(N)$ | $O(\log N)$ recursion stack |
| Alphabetical Listing | `p` | $O(N)$ | $O(N)$ | $O(\log N)$ stack |
| Typo Suggestion | Auto on fail | $O(N \cdot M^2)$ | $O(N \cdot M^2)$ | $O(M^2)$ DP matrix |
| Clear Registry | `x` / `q` | $O(N)$ | $O(N)$ | $O(N)$ freed |

*(Where $N \le 50$ is the number of commands, and $M \le 64$ is string length).*

---

## 5. Compilation & Execution

### Using MSVC (`cl.exe`):
```cmd
cl /W4 /MD main.c /Fe:command_registry.exe
command_registry.exe
```

### Using GCC (MinGW):
```cmd
gcc -Wall -Wextra -std=c99 main.c -o command_registry.exe
./command_registry.exe
```

---

## 6. Standardized Oral Defense (Q&A)

**Q1: Why does In-Order traversal output commands in alphabetical order?**  
*Answer:* By definition of a BST, for any node $X$, all strings in its left subtree are lexicographically smaller than $X$, and all strings in its right subtree are larger than $X$. Visiting `Left -> Node -> Right` guarantees that smaller items are processed before the node, and larger items after, resulting in strict sorted order.

**Q2: How does the program protect against memory leaks when clearing the tree?**  
*Answer:* It uses a recursive post-order traversal (`free_bst`). A parent node cannot be freed before its children, because doing so would destroy the pointers to those children. Post-order traversal ensures both `left` and `right` subtrees are completely freed before calling `free()` on the parent node itself.

**Q3: Does the system approve a command if it is similar?**  
*Answer:* No. The requirement strictly states that only an exact match is approved (`COMMAND APPROVED`). A similar command merely generates a suggestion (`Did you mean: ...?`) while still generating an unauthorized attempt entry in `audit.log`.
