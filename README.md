# DSA Summative Project: 5 Embedded C Engineering Systems

**Course:** Data Structures & Algorithms (DSA)  
**Target Score:** 30 / 30 Points (Mastery / Excellent across all 6 rubric criteria)  
**Implementation Language:** Standard C (C99 / C11 compatible)

---

## Project Structure & Rubric Mapping

| Task Directory | Real-World System | Core Data Structure & Algorithm | Max Points |
| :--- | :--- | :--- | :---: |
| [`task1_event_logger/`](file:///G:/DSA_Projects/dsa_summative_projects/task1_event_logger) | Embedded Device Event Logger | **Doubly Linked List** (Bounded 25-capacity FIFO with cursor navigation) | 5 pts |
| [`task2_command_registry/`](file:///G:/DSA_Projects/dsa_summative_projects/task2_command_registry) | Network Device Command Registry | **Binary Search Tree (BST)** (File loader, In-Order sorting, typo suggestions, audit log) | 5 pts |
| [`task3_server_network/`](file:///G:/DSA_Projects/dsa_summative_projects/task3_server_network) | Server Dependency Analyzer | **Directed Graph & Adjacency Matrix** (Incoming/outgoing queries, dynamic vertex/edge deletion) | 5 pts |
| [`task4_medical_routing/`](file:///G:/DSA_Projects/dsa_summative_projects/task4_medical_routing) | Medical Supply Route Planner | **Weighted Graph & Dijkstra's Algorithm** (Minimum road distance, `parent[]` route reconstruction) | 5 pts |
| [`task5_log_compression/`](file:///G:/DSA_Projects/dsa_summative_projects/task5_log_compression) | Firmware Diagnostic Compressor | **Huffman Coding** (Bitwise compression, header reconstruction, lossless verification) | 5 pts |
| **All Tasks Documentation** | Technical Reports & Viva Prep | Detailed `README.md` in every folder with Big-O analysis and oral exam answers | 5 pts |
| **TOTAL** | | | **30 pts** |

---

## How to Build and Run

### Quick Build All:
Double-click `build_all.bat` or run in terminal:
```cmd
build_all.bat
```

### Build Individual Tasks:
Each task folder is completely independent and contains zero external library dependencies.
For example, to run Task 1:
```cmd
cd task1_event_logger
cl /W4 /MD main.c /Fe:event_logger.exe
event_logger.exe
```
*(Or with GCC)*:
```cmd
gcc -Wall -Wextra -std=c99 main.c -o event_logger.exe
./event_logger.exe
```
