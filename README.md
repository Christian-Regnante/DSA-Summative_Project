# DSA Summative Project (Tasks 1 to 5)

A comprehensive Data Structures and Algorithms practical suite implemented in C (C99), covering Doubly Linked Lists, Binary Search Trees, Directed Graphs, Dijkstra's Shortest Path, and Lossless Huffman Coding.

---

## Project Structure

* **`task1_event_logger/`**: Embedded Device Event Logger using a bounded Doubly Linked List (maximum 25 events, bidirectional navigation, automatic eviction of oldest event, zero memory leaks).
* **`task2_command_registry/`**: Network Device Command Registry using a Binary Search Tree (file loading, in-order alphabetical traversal, 50-command cap, fuzzy typo suggestions, and security `audit.log` logging).
* **`task3_server_dependency/`**: Server Dependency Network Analyzer using a Directed Graph with an Adjacency Matrix (bidirectional queries for incoming/outgoing dependencies, live matrix rendering, dynamic vertex/edge updates).
* **`task4_medical_routing/`**: Emergency Medical Supply Routing using Dijkstra's Algorithm on a weighted road network (multi-hop shortest distance calculation, complete predecessor route reconstruction).
* **`task5_log_compression/`**: Embedded Firmware Log Compression Utility using Huffman Coding (frequency analysis, min-heap priority queue, prefix code generation, bit-packing to `.huff`, lossless decompression, and automated verification).
---

## Quick Build & Test Instructions

To compile and run automated tests for any task:

```bash
# Task 1
cd task1_event_logger && gcc main.c -o event_logger

# Task 2
cd task2_command_registry && gcc main.c -o command_registry

# Task 3
cd task3_server_dependency && gcc main.c -o server_route

# Task 4
cd task4_medical_routing && gcc main.c -o medical_routing

# Task 5
cd task5_log_compression && gcc main.c -o log_compression
```

