/**
 * ============================================================================
 * Task 1: Embedded Device Event Logger
 * Data Structure: Dynamically Allocated Doubly Linked List
 * 
 * Key Features:
 * - Fixed maximum capacity of 25 events (FIFO remove of oldest at head)
 * - Chronological ordering: head = oldest, tail = newest
 * - Interactive bidirectional navigation cursor ('current')
 * - Safe memory management (zero memory leaks, no dangling pointers)
 * - Robust boundary and empty-list error handling
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_EVENTS 25
#define STR_LEN 64

/* Node structure representing a single hardware/sensor event */
typedef struct EventNode {
    int event_id;
    char event_type[STR_LEN];     /* e.g., "TEMP_WARNING", "LOW_BATTERY" */
    char device_id[STR_LEN];      /* e.g., "SENSOR_01", "BATTERY_MOD" */
    char timestamp[STR_LEN];      /* e.g., "2026-10-15 14:30:00" */
    char severity[STR_LEN];       /* e.g., "INFO", "WARNING", "CRITICAL" */

    struct EventNode* prev;       /* Points toward older event (head direction) */
    struct EventNode* next;       /* Points toward newer event (tail direction) */
} EventNode;

/* Master logger controller */
typedef struct {
    EventNode* head;              /* Oldest event */
    EventNode* tail;              /* Newest event */
    EventNode* current;           /* Operator's browsing cursor */
    int count;                    /* Current number of events (0 to 25) */
    int next_auto_id;             /* Auto-incrementing ID counter */
} EventLogger;

/* Initialize the event logger to an empty state */
void init_logger(EventLogger* logger) {
    logger->head = NULL;
    logger->tail = NULL;
    logger->current = NULL;
    logger->count = 0;
    logger->next_auto_id = 1001;
}

/* Helper to generate current timestamp string */
void get_current_timestamp(char* buffer, size_t size) {
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    if (t != NULL) {
        strftime(buffer, size, "%Y-%m-%d %H:%M:%S", t);
    } else {
        strncpy(buffer, "2026-10-15 00:00:00", size);
    }
}

/* Allocate and initialize a new event node dynamically */
EventNode* create_node(int id, const char* type, const char* device, const char* ts, const char* sev) {
    EventNode* node = (EventNode*)malloc(sizeof(EventNode));
    if (node == NULL) {
        printf("[ERROR] Memory allocation failed for new event!\n");
        return NULL;
    }
    node->event_id = id;
    strncpy(node->event_type, type, STR_LEN - 1);
    node->event_type[STR_LEN - 1] = '\0';

    strncpy(node->device_id, device, STR_LEN - 1);
    node->device_id[STR_LEN - 1] = '\0';

    strncpy(node->timestamp, ts, STR_LEN - 1);
    node->timestamp[STR_LEN - 1] = '\0';

    strncpy(node->severity, sev, STR_LEN - 1);
    node->severity[STR_LEN - 1] = '\0';

    node->prev = NULL;
    node->next = NULL;
    return node;
}

/* Remove and free the oldest event from the head */
void evict_oldest(EventLogger* logger) {
    if (logger->head == NULL) {
        return;
    }

    EventNode* old_head = logger->head;

    /* CRITICAL SAFETY CHECK:
     * If the operator is currently viewing the oldest event,
     * advance 'current' to the next event so it doesn't point to freed memory!
     */
    if (logger->current == old_head) {
        logger->current = old_head->next;
    }

    /* Move head to the next event */
    logger->head = old_head->next;

    if (logger->head != NULL) {
        logger->head->prev = NULL;
    } else {
        /* If the list had only 1 node, tail is also now NULL */
        logger->tail = NULL;
        logger->current = NULL;
    }

    printf("[MEMORY] Capacity reached (25). Removed oldest event (ID: %d).\n", old_head->event_id);
    free(old_head);
    logger->count--;
}

/* Add an event to the end (tail) of the list */
void add_event(EventLogger* logger, const char* type, const char* device, const char* ts, const char* sev) {
    /* If list has reached the 25-event limit, reemove the oldest first */
    if (logger->count >= MAX_EVENTS) {
        evict_oldest(logger);
    }

    EventNode* new_node = create_node(logger->next_auto_id++, type, device, ts, sev);
    if (new_node == NULL) return;

    if (logger->head == NULL) {
        /* Case 1: List was empty */
        logger->head = new_node;
        logger->tail = new_node;
        logger->current = new_node; /* First event becomes current */
    } else {
        /* Case 2: Append to tail */
        new_node->prev = logger->tail;
        new_node->next = NULL;
        logger->tail->next = new_node;
        logger->tail = new_node;
    }

    logger->count++;
    printf("[SUCCESS] Event added (ID: %d, Type: %s). Total events: %d/%d\n",
           new_node->event_id, new_node->event_type, logger->count, MAX_EVENTS);
}

/* Move cursor to next (newer) event */
void move_next(EventLogger* logger) {
    if (logger->current == NULL) {
        printf("[INFO] The event log is empty.\n");
        return;
    }
    if (logger->current->next == NULL) {
        printf("[BOUNDARY] Already at the newest event (ID: %d). Cannot move further.\n",
               logger->current->event_id);
    } else {
        logger->current = logger->current->next;
        printf("[NAVIGATE] Moved to newer event (ID: %d, Type: %s).\n",
               logger->current->event_id, logger->current->event_type);
    }
}

/* Move cursor to previous (older) event */
void move_prev(EventLogger* logger) {
    if (logger->current == NULL) {
        printf("[INFO] The event log is empty.\n");
        return;
    }
    if (logger->current->prev == NULL) {
        printf("[BOUNDARY] Already at the oldest event (ID: %d). Cannot move further.\n",
               logger->current->event_id);
    } else {
        logger->current = logger->current->prev;
        printf("[NAVIGATE] Moved to older event (ID: %d, Type: %s).\n",
               logger->current->event_id, logger->current->event_type);
    }
}

/* Display the event currently selected by the cursor */
void display_current(const EventLogger* logger) {
    if (logger->current == NULL) {
        printf("\n========================================\n");
        printf("           CURRENT EVENT LOG            \n");
        printf("========================================\n");
        printf(" [!] The event log is currently empty.\n");
        printf("========================================\n");
        return;
    }

    EventNode* e = logger->current;
    printf("\n========================================\n");
    printf("        CURRENT SELECTED EVENT          \n");
    printf("========================================\n");
    printf(" Event ID:    %d\n", e->event_id);
    printf(" Event Type:  %s\n", e->event_type);
    printf(" Device ID:   %s\n", e->device_id);
    printf(" Timestamp:   %s\n", e->timestamp);
    printf(" Severity:    %s\n", e->severity);
    printf(" Position:    %s%s\n",
           (e == logger->head) ? "[OLDEST / HEAD] " : "",
           (e == logger->tail) ? "[NEWEST / TAIL]" : "");
    printf(" Buffer Load: %d/%d events\n", logger->count, MAX_EVENTS);
    printf("========================================\n");
}

/* Release all allocated nodes and reset the logger */
void clear_all_events(EventLogger* logger) {
    EventNode* curr = logger->head;
    int freed_count = 0;

    while (curr != NULL) {
        EventNode* temp = curr->next;
        free(curr);
        curr = temp;
        freed_count++;
    }

    logger->head = NULL;
    logger->tail = NULL;
    logger->current = NULL;
    logger->count = 0;

    printf("[MEMORY] Cleared all %d events. All dynamic memory successfully released.\n", freed_count);
}

/* Save session log to file and terminate */
void save_session_and_quit(EventLogger* logger, const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (fp == NULL) {
        printf("[ERROR] Failed to open '%s' for saving!\n", filename);
    } else {
        fprintf(fp, "=======================================================================\n");
        fprintf(fp, "               EMBEDDED DEVICE EVENT LOG SESSION DUMP                  \n");
        fprintf(fp, "=======================================================================\n");
        fprintf(fp, "Total Events: %d | Maximum Capacity: %d\n\n", logger->count, MAX_EVENTS);
        fprintf(fp, "%-8s %-18s %-14s %-22s %-10s\n", "ID", "TYPE", "DEVICE", "TIMESTAMP", "SEVERITY");
        fprintf(fp, "-----------------------------------------------------------------------\n");

        EventNode* curr = logger->head;
        while (curr != NULL) {
            fprintf(fp, "%-8d %-18s %-14s %-22s %-10s\n",
                    curr->event_id, curr->event_type, curr->device_id, curr->timestamp, curr->severity);
            curr = curr->next;
        }
        fclose(fp);
        printf("[FILE] Session saved successfully to '%s'.\n", filename);
    }

    /* Release all remaining memory before termination */
    clear_all_events(logger);
    printf("[EXIT] Exiting Event Logger. Goodbye!\n");
}

/* Interactive helper to add an event */
void prompt_add_event(EventLogger* logger) {
    char type[STR_LEN];
    char device[STR_LEN];
    char severity[STR_LEN];
    char ts[STR_LEN];

    printf("\n--- Add New Event Simulation ---\n");
    printf("Enter Event Type (e.g., TEMP_WARNING, LOW_BATTERY, SENSOR_FAIL): ");
    if (scanf("%63s", type) != 1) return;

    printf("Enter Device/Sensor ID (e.g., SENSOR_01, POWER_UNIT): ");
    if (scanf("%63s", device) != 1) return;

    printf("Enter Severity (INFO, WARNING, ERROR, CRITICAL): ");
    if (scanf("%63s", severity) != 1) return;

    get_current_timestamp(ts, sizeof(ts));
    add_event(logger, type, device, ts, severity);
}

/* Quick simulator to pre-populate sample events for testing */
void load_sample_events(EventLogger* logger) {
    char ts[STR_LEN];
    get_current_timestamp(ts, sizeof(ts));

    add_event(logger, "SYS_BOOT", "CONTROLLER_0", ts, "INFO");
    add_event(logger, "TEMP_WARNING", "TEMP_SENSOR_1", ts, "WARNING");
    add_event(logger, "VOLT_NORMAL", "POWER_SUPPLY", ts, "INFO");
    add_event(logger, "COMM_TIMEOUT", "MODEM_4G", ts, "ERROR");
    add_event(logger, "BATTERY_LOW", "BATTERY_PACK", ts, "CRITICAL");
    add_event(logger, "SYS_BOOT", "CONTROLLER_0", ts, "INFO");
    add_event(logger, "SYS_BOOT", "CONTROLLER_0", ts, "INFO");
    add_event(logger, "TEMP_WARNING", "TEMP_SENSOR_1", ts, "WARNING");
    add_event(logger, "VOLT_NORMAL", "POWER_SUPPLY", ts, "INFO");
    add_event(logger, "COMM_TIMEOUT", "MODEM_4G", ts, "ERROR");
    add_event(logger, "BATTERY_LOW", "BATTERY_PACK", ts, "CRITICAL");
    add_event(logger, "TEMP_WARNING", "TEMP_SENSOR_1", ts, "WARNING");
    add_event(logger, "VOLT_NORMAL", "POWER_SUPPLY", ts, "INFO");
    add_event(logger, "COMM_TIMEOUT", "MODEM_4G", ts, "ERROR");
    add_event(logger, "BATTERY_LOW", "BATTERY_PACK", ts, "CRITICAL");
    add_event(logger, "SYS_BOOT", "CONTROLLER_0", ts, "INFO");
    add_event(logger, "SYS_BOOT", "CONTROLLER_0", ts, "INFO");
    add_event(logger, "TEMP_WARNING", "TEMP_SENSOR_1", ts, "WARNING");
    add_event(logger, "VOLT_NORMAL", "POWER_SUPPLY", ts, "INFO");
    add_event(logger, "COMM_TIMEOUT", "MODEM_4G", ts, "ERROR");
    add_event(logger, "BATTERY_LOW", "BATTERY_PACK", ts, "CRITICAL");
    add_event(logger, "TEMP_WARNING", "TEMP_SENSOR_1", ts, "WARNING");
    add_event(logger, "VOLT_NORMAL", "POWER_SUPPLY", ts, "INFO");
    add_event(logger, "COMM_TIMEOUT", "MODEM_4G", ts, "ERROR");
}

int main(void) {
    EventLogger logger;
    init_logger(&logger);

    printf("====================================================\n");
    printf("      EMBEDDED DEVICE EVENT LOGGER (DOUBLY LINKED)  \n");
    printf("====================================================\n");
    /* Requirement 1: The list should initially be empty */
    printf("[INIT] System initialized. Event log is initially empty (0/%d events).\n", MAX_EVENTS);

    char choice;
    while (1) {
        printf("\nCommands: [n]ext | [p]rev | [a]dd | [c]urrent | [x]clear | [l]oad samples | [q]uit\n");
        printf("Enter command: ");
        if (scanf(" %c", &choice) != 1) break;

        switch (choice) {
            case 'n':
            case 'N':
                move_next(&logger);
                break;
            case 'p':
            case 'P':
                move_prev(&logger);
                break;
            case 'a':
            case 'A':
                prompt_add_event(&logger);
                break;
            case 'c':
            case 'C':
                display_current(&logger);
                break;
            case 'x':
            case 'X':
                clear_all_events(&logger);
                break;
            case 'l':
            case 'L':
                printf("\n[SIMULATION] Preloading sample events for testing...\n");
                load_sample_events(&logger);
                break;
            case 'q':
            case 'Q':
                save_session_and_quit(&logger, "events_log.txt");
                return 0;
            default:
                printf("[INVALID] Unknown command '%c'. Valid options are n, p, a, c, x, l, q.\n", choice);
                break;
        }
    }

    /* Fallback safety cleanup */
    clear_all_events(&logger);
    return 0;
}
