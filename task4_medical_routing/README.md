# Task 4: Emergency Medical Supply Routing

## 1. Problem Overview
In emergency medical logistics, transporting blood, oxygen, and vaccines from warehouses to hospitals requires selecting routes that minimize physical travel distance (kilometers) rather than merely minimizing the number of road intersections. 

This project implements **Dijkstra's Shortest Path Algorithm** on an undirected weighted graph and reconstructs the full road-by-road route using a `parent[]` predecessor tree.

---

## 2. Graph Topology & Network Map

* **Vertices ($V$):** 6 Medical facilities and warehouses.
* **Weighted Edges ($E$):** 9 Bidirectional roads with distances in kilometers.

```
                  [Central Warehouse]
                    / (8)         \ (14)
                   v               v
         [Distribution Center]    [District Hospital]
              / (5)       \ (11)      ^   ^   ^
             v             v         /    |    \ (7)
        [Health A]       [Health B] / (9) | (6) \
            \ (4)         / (3)    /     /       \
             v           v        /     /         \
              [Health Center C] -+-----+           +
```

---

## 3. Algorithm: Dijkstra with Route Reconstruction

### How Dijkstra Works:
1. Maintain three arrays:
   - `dist[v]`: Smallest known distance from the start node to node $v$. (Initialized to $0$ for start, $\infty$ for others).
   - `visited[v]`: Flag indicating if node $v$ has been finalized.
   - `parent[v]`: Predecessor node used to reach $v$ on the shortest path.
2. In each iteration, select the unvisited node $u$ with the minimum `dist[u]`.
3. For all neighbors $v$ of $u$, perform **relaxation**:
   $$\text{if } dist[u] + weight(u, v) < dist[v] \implies dist[v] = dist[u] + weight(u, v), \quad parent[v] = u$$
4. Repeat until all nodes are visited or destination is reached.

### Reconstructing the Path:
Since `parent[dest]` points to its predecessor, `parent[parent[dest]]` points to the predecessor before that, etc., we use a clean recursive helper `print_route_recursive`:
```c
void print_route_recursive(const int parent[], int curr, const MedicalNetwork* net) {
    if (parent[curr] == -1) {
        printf("%s", net->names[curr]);
        return;
    }
    print_route_recursive(parent, parent[curr], net);
    printf("  -->  %s", net->names[curr]);
}
```

---

## 4. Time & Space Complexity

| Operation | Complexity | Description |
| :--- | :---: | :---: |
| Finding Min Distance Vertex | $O(V)$ | Linear scan across unvisited vertices |
| Neighbor Relaxation | $O(V)$ | Iterating across columns in adjacency row |
| Total Dijkstra Execution | $O(V^2)$ | Optimal and compact for dense/small graphs ($V \le 10$) |
| Path Reconstruction | $O(V)$ | Unwinding the predecessor chain |
| Distance Matrix Space | $O(V^2)$ | 2D matrix storing road distances |
| Auxiliary Arrays Space | $O(V)$ | `dist[]`, `visited[]`, and `parent[]` arrays |

---

## 5. Compilation & Execution

### Using MSVC (`cl.exe`):
```cmd
cl /W4 /MD main.c /Fe:medical_routing.exe
medical_routing.exe
```

### Using GCC (MinGW):
```cmd
gcc -Wall -Wextra -std=c99 main.c -o medical_routing.exe
./medical_routing.exe
```

---

## 6. Standardized Oral Defense (Q&A)

**Q1: Why does Dijkstra's algorithm guarantee the globally shortest path?**  
*Answer:* Dijkstra relies on the greedy choice property and optimal substructure. Because all road weights are non-negative, any subpath of an optimal path must also be optimal. By always processing the unvisited node with the smallest cumulative distance next, we guarantee that once a node is marked visited, its minimum distance cannot be improved by taking a detour through larger paths.

**Q2: How does the program satisfy the rubric requirement that "displaying only the final distance is not sufficient"?**  
*Answer:* We store a `parent[]` array. During each edge relaxation step (`dist[u] + weight < dist[v]`), we set `parent[v] = u`. After the algorithm concludes, we backtrack through the `parent` chain from destination back to start, recursively displaying the exact sequence of facilities visited.

**Q3: How are edge cases like choosing the same start and destination handled?**  
*Answer:* If `start == dest`, the algorithm detects this immediately before running, reporting a total distance of `0 km` and indicating that the vehicle is already at its target facility, avoiding unnecessary computations.
