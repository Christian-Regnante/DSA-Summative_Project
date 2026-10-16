# Task 3: Server Dependency Network Analyzer

## 1. Problem Overview
Modern IT infrastructure relies on microservices and multi-tier architectures where servers communicate with one another. When an administrator needs to perform maintenance or isolate a fault, they must quickly know:
1. Which servers does a given server rely on? (Outgoing dependencies)
2. Which servers will break if this server goes down? (Incoming dependencies)

This project models the server topology using a **Directed Graph** represented via an **Adjacency Matrix** in C.

---

## 2. Data Structure Design & Architecture

### Graph Representation: Adjacency Matrix
* **Vertices ($V$):** Each server is assigned an integer index ($0, 1, 2, \dots$).
* **Directed Edges ($E$):** $A \rightarrow B$ indicates that Server $A$ depends on Server $B$.
* **Matrix Encoding:**
  * `matrix[A][B] = 1`: Direct dependency exists from $A$ to $B$.
  * `matrix[A][B] = 0`: No direct dependency.

```
                      [0] Web  [1] API  [2] Auth  [3] DB  [4] Cache  [5] Backup
[0] Web Server           0        1        1        0        0          0
[1] API Server           0        0        0        1        1          0
[2] Auth Server          0        0        0        1        0          0
[3] Database Server      0        0        0        0        0          1
[4] Cache Server         0        0        0        1        0          0
[5] Backup Server        0        0        0        0        0          0
```

---

## 3. Key Implementation Details

* **Outgoing Query (What does $X$ depend on?):** Scans row $X$ of the matrix. Every column $Y$ where `matrix[X][Y] == 1` is an outgoing dependency.
* **Incoming Query (Who depends on $X$?):** Scans column $X$ of the matrix. Every row $Z$ where `matrix[Z][X] == 1` is a dependent service.
* **Vertex Deletion with Matrix Compaction:** When a server at index $k$ is removed:
  1. Names are shifted up starting at index $k$.
  2. Matrix rows are shifted up (`matrix[i][j] = matrix[i+1][j]`).
  3. Matrix columns are shifted left (`matrix[i][j] = matrix[i][j+1]`).
  4. The boundary cells are cleared to 0.
  5. This completely removes all connected edges automatically without leaving gaps or corrupted indices.

---

## 4. Time & Space Complexity

| Operation | Command | Time Complexity | Space Complexity |
| :--- | :---: | :---: | :---: |
| Dependency Query | `q` | $O(V)$ | $O(1)$ |
| Check Direct Edge | Internal | $O(1)$ | $O(1)$ |
| Add Dependency | `c` | $O(1)$ after index lookup | $O(1)$ |
| Delete Dependency | `d` | $O(1)$ after index lookup | $O(1)$ |
| Add Server | `a` | $O(V)$ name scan | $O(1)$ |
| Remove Server | `r` | $O(V^2)$ matrix shift | $O(1)$ auxiliary |
| Display Matrix | `m` | $O(V^2)$ | $O(1)$ auxiliary |
| Total Graph Storage | - | - | $O(V^2)$ space |

*(Where $V \le 20$ is the number of servers).*

---

## 5. Compilation & Execution (Linux)

```bash
gcc -Wall -Wextra -std=c99 -pedantic main.c -o server_network
./server_network
```

---

## 6. Standardized Oral Defense (Q&A)

**Q1: Why choose an Adjacency Matrix over an Adjacency List for this project?**  
*Answer:* An Adjacency Matrix offers instantaneous $O(1)$ edge existence checks (`matrix[u][v] == 1`), straightforward row/column iteration for bidirectional dependency reporting, and very simple visualization for human operators. Since the maximum number of servers is bounded (20), the $O(V^2)$ memory footprint is minuscule (under 2 KB).

**Q2: How do you identify which services will be impacted if a server fails?**  
*Answer:* You inspect the **column** corresponding to that server. In our matrix convention where `matrix[row][col] == 1` means `row -> col`, any row with a 1 in column $X$ represents a service that directly relies on $X$.

**Q3: How does the program ensure graph consistency when a server is removed?**  
*Answer:* Removing a vertex requires shifting all subsequent rows up and columns left. This compacts the matrix, maintaining contiguous $0 \dots (V-1)$ indices and simultaneously severing all incoming and outgoing connections without leaving dangling pointers.
