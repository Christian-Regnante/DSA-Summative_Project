/**
 * ============================================================================
 * Task 4: Emergency Medical Supply Routing
 * Data Structure: Weighted Undirected Graph & Dijkstra's Algorithm
 * 
 * Key Features:
 * - Medical facilities modeled as vertices; roads as weighted edges (distances in km)
 * - 2D Distance Adjacency Matrix with INF (999999) representing unconnected locations
 * - Preloaded with 6 medical centers and 9 real road distances
 * - Dijkstra's algorithm for computing guaranteed minimum-distance paths
 * - Full route reconstruction using a parent[] breadcrumb trail (not just distance)
 * - Comprehensive input validation: same source/destination, non-existent places, unreachable nodes
 * ============================================================================
 */

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LOCATIONS 10
#define NAME_LEN 64
#define INF 999999

/* Weighted graph structure */
typedef struct {
    char names[MAX_LOCATIONS][NAME_LEN];
    int matrix[MAX_LOCATIONS][MAX_LOCATIONS];
    int count;
} MedicalNetwork;

/* Initialize network */
void init_network(MedicalNetwork* net) {
    net->count = 0;
    for (int i = 0; i < MAX_LOCATIONS; i++) {
        for (int j = 0; j < MAX_LOCATIONS; j++) {
            net->matrix[i][j] = (i == j) ? 0 : INF;
        }
        net->names[i][0] = '\0';
    }
}

/* Helper to trim whitespace */
void trim_whitespace(char* str) {
    char* end;
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) return;
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
}

/* Find location index by name (-1 if not found, case-insensitive) */
int find_location(const MedicalNetwork* net, const char* name) {
    for (int i = 0; i < net->count; i++) {
#ifdef _WIN32
        if (_stricmp(net->names[i], name) == 0) return i;
#else
        if (strcasecmp(net->names[i], name) == 0) return i;
#endif
    }
    return -1;
}

/* Add a medical location vertex */
int add_location(MedicalNetwork* net, const char* name) {
    if (net->count >= MAX_LOCATIONS) {
        printf("[CAPACITY] Reached maximum locations limit of %d.\n", MAX_LOCATIONS);
        return -1;
    }
    int existing = find_location(net, name);
    if (existing != -1) return existing;

    int idx = net->count;
    strncpy(net->names[idx], name, NAME_LEN - 1);
    net->names[idx][NAME_LEN - 1] = '\0';
    net->count++;
    return idx;
}

/* Add bidirectional road edge with distance in km */
void add_road(MedicalNetwork* net, const char* locA, const char* locB, int dist_km) {
    if (dist_km <= 0) {
        printf("[ERROR] Road distance must be positive! (%d km given)\n", dist_km);
        return;
    }

    int u = add_location(net, locA);
    int v = add_location(net, locB);

    if (u != -1 && v != -1) {
        net->matrix[u][v] = dist_km;
        net->matrix[v][u] = dist_km; /* Undirected road */
    }
}

/* Display all medical locations */
void display_locations(const MedicalNetwork* net) {
    printf("\n--- REGISTERED MEDICAL FACILITIES (%d total) ---\n", net->count);
    for (int i = 0; i < net->count; i++) {
        printf("  [%d] %s\n", i + 1, net->names[i]);
    }
    printf("-------------------------------------------------\n");
}

/* Display complete distance matrix */
void display_distance_matrix(const MedicalNetwork* net) {
    printf("\n=================== ROAD DISTANCE MATRIX (KM) ===================\n");
    printf("%-24s", "Facility");
    for (int j = 0; j < net->count; j++) {
        printf("[%d]   ", j + 1);
    }
    printf("\n-----------------------------------------------------------------\n");

    for (int i = 0; i < net->count; i++) {
        printf("[%d] %-20s", i + 1, net->names[i]);
        for (int j = 0; j < net->count; j++) {
            if (net->matrix[i][j] == INF) {
                printf(" --   ");
            } else {
                printf("%3d   ", net->matrix[i][j]);
            }
        }
        printf("\n");
    }
    printf("=================================================================\n");
}

/* Helper to reconstruct and print the path recursively */
void print_route_recursive(const int parent[], int current, const MedicalNetwork* net) {
    if (parent[current] == -1) {
        printf("%s", net->names[current]);
        return;
    }
    print_route_recursive(parent, parent[current], net);
    printf("  -->  %s", net->names[current]);
}

/* Dijkstra's Algorithm implementation with full path reconstruction */
void find_shortest_route(const MedicalNetwork* net, const char* start_name, const char* dest_name) {
    int start = find_location(net, start_name);
    int dest = find_location(net, dest_name);

    /* Input validation */
    if (start == -1) {
        printf("[ERROR] Starting location '%s' not recognized!\n", start_name);
        return;
    }
    if (dest == -1) {
        printf("[ERROR] Destination location '%s' not recognized!\n", dest_name);
        return;
    }

    /* Edge Case: Same start and destination */
    if (start == dest) {
        printf("\n=======================================================\n");
        printf("               OPTIMAL DELIVERY ROUTE                  \n");
        printf("=======================================================\n");
        printf(" Shortest Route:\n");
        printf("   %s (Delivery vehicle is already at the destination!)\n\n", net->names[start]);
        printf(" Total Distance: 0 km\n");
        printf("=======================================================\n");
        return;
    }

    int dist[MAX_LOCATIONS];
    int visited[MAX_LOCATIONS];
    int parent[MAX_LOCATIONS];

    /* 1. Initialization */
    for (int i = 0; i < net->count; i++) {
        dist[i] = INF;
        visited[i] = 0;
        parent[i] = -1;
    }
    dist[start] = 0;

    /* 2. Main Dijkstra greedy relaxation loop */
    for (int step = 0; step < net->count; step++) {
        /* Find unvisited vertex with smallest distance */
        int min_val = INF;
        int u = -1;

        for (int i = 0; i < net->count; i++) {
            if (!visited[i] && dist[i] < min_val) {
                min_val = dist[i];
                u = i;
            }
        }

        /* If remaining nodes are unreachable */
        if (u == -1) break;

        visited[u] = 1;

        /* Early exit if destination reached */
        if (u == dest) break;

        /* Relax neighbors */
        for (int v = 0; v < net->count; v++) {
            if (!visited[v] && net->matrix[u][v] != INF) {
                int new_dist = dist[u] + net->matrix[u][v];
                if (new_dist < dist[v]) {
                    dist[v] = new_dist;
                    parent[v] = u; /* Record predecessor to reconstruct route */
                }
            }
        }
    }

    /* 3. Output results */
    if (dist[dest] == INF) {
        printf("\n[ALERT] No road route exists between '%s' and '%s'!\n",
               net->names[start], net->names[dest]);
        return;
    }

    printf("\n=======================================================\n");
    printf("               OPTIMAL DELIVERY ROUTE                  \n");
    printf("=======================================================\n");
    printf(" Shortest Route:\n   ");
    print_route_recursive(parent, dest, net);
    printf("\n\n Total Distance: %d km\n", dist[dest]);
    printf("=======================================================\n");
}

/* Preload the standard medical network from the problem description */
void preload_network(MedicalNetwork* net) {
    add_road(net, "Central Warehouse", "Distribution Center", 8);
    add_road(net, "Central Warehouse", "District Hospital", 14);
    add_road(net, "Distribution Center", "Health Center A", 5);
    add_road(net, "Distribution Center", "Health Center B", 11);
    add_road(net, "Health Center A", "District Hospital", 7);
    add_road(net, "Health Center B", "Health Center C", 3);
    add_road(net, "Health Center C", "District Hospital", 6);
    add_road(net, "Health Center B", "District Hospital", 9);
    add_road(net, "Health Center A", "Health Center C", 4);
}

/* Helper to read a string line safely */
void read_line(char* buffer, size_t size) {
    if (fgets(buffer, (int)size, stdin) != NULL) {
        trim_whitespace(buffer);
    } else {
        buffer[0] = '\0';
    }
}

int main(void) {
    MedicalNetwork net;
    init_network(&net);

    printf("====================================================\n");
    printf("   EMERGENCY MEDICAL SUPPLY ROUTING (DIJKSTRA)      \n");
    printf("====================================================\n");
    printf("Loading regional medical road network...\n");
    preload_network(&net);

    int option;
    char start[NAME_LEN];
    char dest[NAME_LEN];

    while (1) {
        printf("\nDispatcher Menu:\n");
        printf("  1. Display medical facilities\n");
        printf("  2. Find shortest delivery route\n");
        printf("  3. Display road distance matrix\n");
        printf("  4. Exit\n");
        printf("Select option (1-4): ");

        if (scanf("%d", &option) != 1) {
            printf("[INVALID] Please enter a valid number.\n");
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            continue;
        }
        int ch_flush;
        while ((ch_flush = getchar()) != '\n' && ch_flush != EOF); /* Consume trailing newline */

        switch (option) {
            case 1:
                display_locations(&net);
                break;

            case 2:
                printf("\nEnter Starting Facility: ");
                read_line(start, sizeof(start));
                printf("Enter Destination Facility: ");
                read_line(dest, sizeof(dest));

                if (strlen(start) > 0 && strlen(dest) > 0) {
                    find_shortest_route(&net, start, dest);
                }
                break;

            case 3:
                display_distance_matrix(&net);
                break;

            case 4:
                printf("[EXIT] Medical Routing System closed. Drive safely!\n");
                return 0;

            default:
                printf("[INVALID] Option %d not recognized. Choose 1 to 4.\n", option);
                break;
        }
    }

    return 0;
}
