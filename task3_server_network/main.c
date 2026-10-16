/**
 * ============================================================================
 * Task 3: Server Dependency Network Analyzer
 * Data Structure: Directed Graph with Adjacency Matrix
 * 
 * Key Features:
 * - Servers modeled as vertices; directed dependencies as edges (A -> B means A needs B)
 * - 2D Adjacency Matrix representation: matrix[A][B] = 1
 * - Preloaded with 6 core enterprise servers and 7 initial dependencies
 * - Dual query mode: Direct dependencies (row scan) & Dependent services (col scan)
 * - Dynamic vertex removal with full row/column shifting to preserve graph consistency
 * - Complete input validation and edge-case guards (empty graph, duplicates, non-existent)
 * ============================================================================
 */

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_SERVERS 20
#define NAME_LEN 64

/* Directed graph structure backed by an adjacency matrix */
typedef struct {
    char names[MAX_SERVERS][NAME_LEN];
    int matrix[MAX_SERVERS][MAX_SERVERS];
    int num_servers;
} ServerGraph;

/* Initialize an empty server graph */
void init_graph(ServerGraph* g) {
    g->num_servers = 0;
    for (int i = 0; i < MAX_SERVERS; i++) {
        for (int j = 0; j < MAX_SERVERS; j++) {
            g->matrix[i][j] = 0;
        }
        g->names[i][0] = '\0';
    }
}

/* Helper to trim leading and trailing whitespace */
void trim_whitespace(char* str) {
    char* end;
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) return;
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
}

/* Find index of a server by name (-1 if not found, case-insensitive) */
int find_server(const ServerGraph* g, const char* name) {
    for (int i = 0; i < g->num_servers; i++) {
#ifdef _WIN32
        if (_stricmp(g->names[i], name) == 0) return i;
#else
        if (strcasecmp(g->names[i], name) == 0) return i;
#endif
    }
    return -1;
}

/* Add a new server vertex to the graph */
int add_server(ServerGraph* g, const char* name) {
    if (g->num_servers >= MAX_SERVERS) {
        printf("[CAPACITY] Cannot add server '%s'. Maximum capacity of %d servers reached.\n",
               name, MAX_SERVERS);
        return -1;
    }

    if (find_server(g, name) != -1) {
        printf("[DUPLICATE] Server '%s' already exists in the network!\n", name);
        return -1;
    }

    int idx = g->num_servers;
    strncpy(g->names[idx], name, NAME_LEN - 1);
    g->names[idx][NAME_LEN - 1] = '\0';

    /* Initialize its row and column to 0 */
    for (int i = 0; i < MAX_SERVERS; i++) {
        g->matrix[idx][i] = 0;
        g->matrix[i][idx] = 0;
    }

    g->num_servers++;
    printf("[SUCCESS] Added server [%d]: '%s'\n", idx, name);
    return idx;
}

/* Create a directed dependency: from_server -> to_server (from depends on to) */
int add_dependency(ServerGraph* g, const char* from_name, const char* to_name) {
    int u = find_server(g, from_name);
    int v = find_server(g, to_name);

    if (u == -1) {
        printf("[ERROR] Source server '%s' does not exist!\n", from_name);
        return 0;
    }
    if (v == -1) {
        printf("[ERROR] Destination server '%s' does not exist!\n", to_name);
        return 0;
    }
    if (u == v) {
        printf("[WARNING] A server cannot depend directly on itself!\n");
        return 0;
    }
    if (g->matrix[u][v] == 1) {
        printf("[DUPLICATE] Dependency '%s -> %s' already exists.\n", from_name, to_name);
        return 0;
    }

    g->matrix[u][v] = 1;
    printf("[CONNECTED] Dependency created: %s -> %s\n", g->names[u], g->names[v]);
    return 1;
}

/* Delete a directed dependency: from_server -> to_server */
int remove_dependency(ServerGraph* g, const char* from_name, const char* to_name) {
    int u = find_server(g, from_name);
    int v = find_server(g, to_name);

    if (u == -1 || v == -1) {
        printf("[ERROR] One or both servers do not exist in the network!\n");
        return 0;
    }
    if (g->matrix[u][v] == 0) {
        printf("[ERROR] No existing dependency from '%s' to '%s'.\n", from_name, to_name);
        return 0;
    }

    g->matrix[u][v] = 0;
    printf("[DELETED] Dependency removed: %s -X-> %s\n", g->names[u], g->names[v]);
    return 1;
}

/* Remove a server and shift matrix rows & columns to preserve alignment */
int remove_server(ServerGraph* g, const char* name) {
    int k = find_server(g, name);
    if (k == -1) {
        printf("[ERROR] Cannot remove server '%s': Not found in network.\n", name);
        return 0;
    }

    printf("[REMOVING] Server '%s' (index %d) and all connected relationships...\n", g->names[k], k);

    /* 1. Shift names array */
    for (int i = k; i < g->num_servers - 1; i++) {
        strcpy(g->names[i], g->names[i + 1]);
    }

    /* 2. Shift matrix rows UP */
    for (int i = k; i < g->num_servers - 1; i++) {
        for (int j = 0; j < g->num_servers; j++) {
            g->matrix[i][j] = g->matrix[i + 1][j];
        }
    }

    /* 3. Shift matrix columns LEFT */
    for (int j = k; j < g->num_servers - 1; j++) {
        for (int i = 0; i < g->num_servers; i++) {
            g->matrix[i][j] = g->matrix[i][j + 1];
        }
    }

    /* 4. Zero out the old boundary row and column */
    int last = g->num_servers - 1;
    for (int i = 0; i < MAX_SERVERS; i++) {
        g->matrix[last][i] = 0;
        g->matrix[i][last] = 0;
    }

    g->num_servers--;
    printf("[SUCCESS] Server removed. Total active servers: %d\n", g->num_servers);
    return 1;
}

/* Query dependencies for a specified server */
void query_dependencies(const ServerGraph* g, const char* name) {
    if (g->num_servers == 0) {
        printf("[INFO] The server network is currently empty.\n");
        return;
    }

    int idx = find_server(g, name);
    if (idx == -1) {
        printf("[ERROR] Server '%s' does not exist in the network.\n", name);
        return;
    }

    printf("\n======================================================\n");
    printf("         DEPENDENCY REPORT: %s\n", g->names[idx]);
    printf("======================================================\n");

    /* 1. Outgoing edges (Direct dependencies: scan row) */
    printf("\n[1] Direct Dependencies (%s relies on):\n", g->names[idx]);
    int direct_count = 0;
    for (int col = 0; col < g->num_servers; col++) {
        if (g->matrix[idx][col] == 1) {
            printf("    -> %s\n", g->names[col]);
            direct_count++;
        }
    }
    if (direct_count == 0) {
        printf("    (None - standalone service)\n");
    }

    /* 2. Incoming edges (Services depending on this server: scan column) */
    printf("\n[2] Services Depending on %s:\n", g->names[idx]);
    int dependent_count = 0;
    for (int row = 0; row < g->num_servers; row++) {
        if (g->matrix[row][idx] == 1) {
            printf("    <- %s\n", g->names[row]);
            dependent_count++;
        }
    }
    if (dependent_count == 0) {
        printf("    (None - no other services rely on this server)\n");
    }
    printf("======================================================\n");
}

/* Display the formatted adjacency matrix */
void display_matrix(const ServerGraph* g) {
    if (g->num_servers == 0) {
        printf("[INFO] The server network is empty. Nothing to display.\n");
        return;
    }

    printf("\n================== ADJACENCY MATRIX ==================\n");
    printf("Rows = Source (Depends on) | Columns = Target (Required Service)\n\n");

    /* Header numbers */
    printf("%-24s", "Server Name");
    for (int j = 0; j < g->num_servers; j++) {
        printf("[%2d] ", j);
    }
    printf("\n");

    /* Divider */
    printf("------------------------");
    for (int j = 0; j < g->num_servers; j++) {
        printf("-----");
    }
    printf("\n");

    /* Rows */
    for (int i = 0; i < g->num_servers; i++) {
        printf("[%2d] %-20s", i, g->names[i]);
        for (int j = 0; j < g->num_servers; j++) {
            printf("  %d  ", g->matrix[i][j]);
        }
        printf("\n");
    }
    printf("======================================================\n");
}

/* Pre-populate graph with the 6 servers and 7 edges from the problem */
void preload_network(ServerGraph* g) {
    add_server(g, "Web Server");
    add_server(g, "API Server");
    add_server(g, "Authentication Server");
    add_server(g, "Database Server");
    add_server(g, "Cache Server");
    add_server(g, "Backup Server");

    add_dependency(g, "Web Server", "API Server");
    add_dependency(g, "Web Server", "Authentication Server");
    add_dependency(g, "API Server", "Database Server");
    add_dependency(g, "API Server", "Cache Server");
    add_dependency(g, "Authentication Server", "Database Server");
    add_dependency(g, "Cache Server", "Database Server");
    add_dependency(g, "Database Server", "Backup Server");
}

/* Read a full line from standard input cleanly */
void read_line(char* buffer, size_t size) {
    if (fgets(buffer, (int)size, stdin) != NULL) {
        trim_whitespace(buffer);
    } else {
        buffer[0] = '\0';
    }
}

int main(void) {
    ServerGraph graph;
    init_graph(&graph);

    printf("====================================================\n");
    printf("    SERVER DEPENDENCY NETWORK ANALYZER (GRAPH)     \n");
    printf("====================================================\n");
    printf("Preloading corporate server topology...\n");
    preload_network(&graph);

    char choice;
    char name1[NAME_LEN];
    char name2[NAME_LEN];

    while (1) {
        printf("\nCommands: [a]dd server | [r]emove server | [c]reate dependency | [d]elete dependency\n");
        printf("          [q]uery dependencies | [m]atrix display | [x]exit\n");
        printf("Enter command: ");
        if (scanf(" %c", &choice) != 1) break;
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF); /* Consume trailing characters on line */

        switch (choice) {
            case 'a':
            case 'A':
                printf("Enter new server name: ");
                read_line(name1, sizeof(name1));
                if (strlen(name1) > 0) add_server(&graph, name1);
                break;

            case 'r':
            case 'R':
                printf("Enter server name to remove: ");
                read_line(name1, sizeof(name1));
                if (strlen(name1) > 0) remove_server(&graph, name1);
                break;

            case 'c':
            case 'C':
                printf("Enter dependent server (Source): ");
                read_line(name1, sizeof(name1));
                printf("Enter required server (Target): ");
                read_line(name2, sizeof(name2));
                if (strlen(name1) > 0 && strlen(name2) > 0) {
                    add_dependency(&graph, name1, name2);
                }
                break;

            case 'd':
            case 'D':
                printf("Enter dependent server (Source): ");
                read_line(name1, sizeof(name1));
                printf("Enter required server (Target): ");
                read_line(name2, sizeof(name2));
                if (strlen(name1) > 0 && strlen(name2) > 0) {
                    remove_dependency(&graph, name1, name2);
                }
                break;

            case 'q':
            case 'Q':
                printf("Enter server name to query: ");
                read_line(name1, sizeof(name1));
                if (strlen(name1) > 0) query_dependencies(&graph, name1);
                break;

            case 'm':
            case 'M':
                display_matrix(&graph);
                break;

            case 'x':
            case 'X':
                printf("[EXIT] Terminating Server Dependency Analyzer. Goodbye!\n");
                return 0;

            default:
                printf("[INVALID] Unknown option '%c'. Valid options: a, r, c, d, q, m, x.\n", choice);
                break;
        }
    }

    return 0;
}
