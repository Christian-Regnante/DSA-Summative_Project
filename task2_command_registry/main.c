/**
 * ============================================================================
 * Task 2: Network Device Command Registry
 * Data Structure: Dynamically Allocated Binary Search Tree (BST)
 * 
 * Key Features:
 * - Approved commands loaded from commands.txt into an alphabetically ordered BST
 * - Strict 50-command maximum capacity limit enforced
 * - Fast O(log N) exact command verification ('v')
 * - Fuzzy typo suggestion ("Did you mean: ...?") using Levenshtein distance
 * - Security audit logging of unapproved attempts into audit.log with timestamps
 * - Alphabetical command listing via recursive in-order traversal ('p')
 * - Safe memory management via recursive post-order deallocation ('x', 'q')
 * ============================================================================
 */

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define MAX_COMMANDS 50
#define CMD_LEN 64
#define AUDIT_LOG_FILE "audit.log"
#define DEFAULT_COMMAND_FILE "commands.txt"

/* BST node structure storing a single approved command */
typedef struct CommandNode {
    char command[CMD_LEN];
    struct CommandNode* left;
    struct CommandNode* right;
} CommandNode;

/* Master registry controller */
typedef struct {
    CommandNode* root;
    int count;
} CommandRegistry;

/* Initialize registry to empty */
void init_registry(CommandRegistry* reg) {
    reg->root = NULL;
    reg->count = 0;
}

/* Helper to trim leading/trailing whitespace and newlines */
void trim_whitespace(char* str) {
    char* end;
    /* Trim leading spaces */
    while (isspace((unsigned char)*str)) str++;

    if (*str == 0) return;

    /* Trim trailing spaces and newlines */
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;

    end[1] = '\0';
}

/* Allocate a new BST node */
CommandNode* create_node(const char* cmd) {
    CommandNode* node = (CommandNode*)malloc(sizeof(CommandNode));
    if (node == NULL) {
        printf("[ERROR] Failed to allocate memory for command '%s'!\n", cmd);
        return NULL;
    }
    strncpy(node->command, cmd, CMD_LEN - 1);
    node->command[CMD_LEN - 1] = '\0';
    node->left = NULL;
    node->right = NULL;
    return node;
}

/* Insert a command into the BST with capacity and duplicate checking */
CommandNode* insert_bst(CommandNode* root, const char* cmd, int* count, int* inserted) {
    if (root == NULL) {
        if (*count >= MAX_COMMANDS) {
            printf("[CAPACITY] Limit reached (50). Skipping '%s'\n", cmd);
            *inserted = 0;
            return NULL;
        }
        CommandNode* new_node = create_node(cmd);
        if (new_node != NULL) {
            (*count)++;
            *inserted = 1;
        }
        return new_node;
    }

    int cmp = strcmp(cmd, root->command);

    if (cmp < 0) {
        root->left = insert_bst(root->left, cmd, count, inserted);
    } else if (cmp > 0) {
        root->right = insert_bst(root->right, cmd, count, inserted);
    } else {
        /* Duplicate found: ignore to maintain unique set */
        *inserted = 0;
    }

    return root;
}

/* Search for exact match in BST */
CommandNode* search_exact(CommandNode* root, const char* cmd) {
    if (root == NULL) return NULL;

    int cmp = strcmp(cmd, root->command);
    if (cmp == 0) {
        return root;
    } else if (cmp < 0) {
        return search_exact(root->left, cmd);
    } else {
        return search_exact(root->right, cmd);
    }
}

/* In-order traversal to print commands in alphabetical order */
void inorder_print(CommandNode* root, int* serial) {
    if (root == NULL) return;

    inorder_print(root->left, serial);
    printf("  [%2d] %s\n", (*serial)++, root->command);
    inorder_print(root->right, serial);
}

/* Recursive post-order traversal to free all nodes */
CommandNode* free_bst(CommandNode* root, int* freed_count) {
    if (root == NULL) return NULL;

    free_bst(root->left, freed_count);
    free_bst(root->right, freed_count);

    free(root);
    (*freed_count)++;
    return NULL;
}

/* Calculate Levenshtein edit distance between two strings */
int min3(int a, int b, int c) {
    int m = a;
    if (b < m) m = b;
    if (c < m) m = c;
    return m;
}

int edit_distance(const char* s1, const char* s2) {
    int len1 = (int)strlen(s1);
    int len2 = (int)strlen(s2);
    int dp[CMD_LEN + 1][CMD_LEN + 1];

    for (int i = 0; i <= len1; i++) dp[i][0] = i;
    for (int j = 0; j <= len2; j++) dp[0][j] = j;

    for (int i = 1; i <= len1; i++) {
        for (int j = 1; j <= len2; j++) {
            if (toupper((unsigned char)s1[i - 1]) == toupper((unsigned char)s2[j - 1])) {
                dp[i][j] = dp[i - 1][j - 1];
            } else {
                dp[i][j] = 1 + min3(dp[i - 1][j],      /* Deletion */
                                    dp[i][j - 1],      /* Insertion */
                                    dp[i - 1][j - 1]); /* Substitution */
            }
        }
    }
    return dp[len1][len2];
}

/* Traverse BST to find the most similar command */
void find_best_suggestion(CommandNode* root, const char* target, char* best_match, int* min_dist) {
    if (root == NULL) return;

    int dist = edit_distance(target, root->command);
    if (dist < *min_dist) {
        *min_dist = dist;
        strncpy(best_match, root->command, CMD_LEN - 1);
        best_match[CMD_LEN - 1] = '\0';
    }

    find_best_suggestion(root->left, target, best_match, min_dist);
    find_best_suggestion(root->right, target, best_match, min_dist);
}

/* Record unauthorized/unknown command into audit.log */
void log_unauthorized_attempt(const char* cmd) {
    FILE* fp = fopen(AUDIT_LOG_FILE, "a");
    if (fp == NULL) {
        printf("[ERROR] Could not open audit log '%s' for writing!\n", AUDIT_LOG_FILE);
        return;
    }

    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    char time_str[32];
    if (t != NULL) {
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", t);
    } else {
        strncpy(time_str, "2026-10-15 00:00:00", sizeof(time_str));
    }

    fprintf(fp, "%s | UNKNOWN | %s\n", time_str, cmd);
    fclose(fp);
    printf("[SECURITY] Incident recorded in '%s': %s | UNKNOWN | %s\n",
           AUDIT_LOG_FILE, time_str, cmd);
}

/* Load commands from text file */
void load_commands_from_file(CommandRegistry* reg, const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("[WARNING] Command file '%s' not found. Starting with empty registry.\n", filename);
        return;
    }

    char line[CMD_LEN];
    int loaded = 0;
    int skipped_dupes = 0;

    while (fgets(line, sizeof(line), fp) != NULL) {
        trim_whitespace(line);
        if (strlen(line) == 0) continue;

        int inserted = 0;
        reg->root = insert_bst(reg->root, line, &reg->count, &inserted);
        if (inserted) {
            loaded++;
        } else {
            skipped_dupes++;
        }

        if (reg->count >= MAX_COMMANDS) {
            printf("[INFO] Reached maximum capacity of %d commands.\n", MAX_COMMANDS);
            break;
        }
    }

    fclose(fp);
    printf("[LOADED] Successfully loaded %d approved commands (Skipped %d duplicates/overflows).\n",
           loaded, skipped_dupes);
}

/* Verify a user-entered command */
void verify_command(CommandRegistry* reg, const char* input_cmd) {
    if (reg->root == NULL) {
        printf("\n[ALERT] Command registry is empty! Please load or add commands first.\n");
        log_unauthorized_attempt(input_cmd);
        return;
    }

    CommandNode* match = search_exact(reg->root, input_cmd);
    if (match != NULL) {
        printf("\n========================================\n");
        printf("  VERIFICATION RESULT: COMMAND APPROVED \n");
        printf("========================================\n");
        printf(" Command '%s' is an approved device instruction.\n", match->command);
        return;
    }

    /* Command not found: check for suggestions */
    printf("\n========================================\n");
    printf("  VERIFICATION RESULT: UNKNOWN COMMAND  \n");
    printf("========================================\n");

    char best_match[CMD_LEN] = "";
    int min_dist = 999;
    find_best_suggestion(reg->root, input_cmd, best_match, &min_dist);

    /* If edit distance is small (1 or 2 edits), suggest it */
    if (min_dist <= 2 && strlen(best_match) > 0) {
        printf(" Did you mean: %s?\n", best_match);
    }

    /* Log incident */
    log_unauthorized_attempt(input_cmd);
}

/* Clear registry memory */
void clear_registry(CommandRegistry* reg) {
    int freed = 0;
    reg->root = free_bst(reg->root, &freed);
    reg->count = 0;
    printf("[MEMORY] Command registry cleared. Freed %d nodes.\n", freed);
}

int main(void) {
    CommandRegistry reg;
    init_registry(&reg);

    printf("====================================================\n");
    printf("     NETWORK DEVICE COMMAND REGISTRY (BST SYSTEM)   \n");
    printf("====================================================\n");
    load_commands_from_file(&reg, DEFAULT_COMMAND_FILE);

    char choice;
    char buffer[CMD_LEN];

    while (1) {
        printf("\nCommands: [v]erify | [p]rint alphabetical | [s]ize count | [x]clear | [q]uit\n");
        printf("Enter command: ");
        if (scanf(" %c", &choice) != 1) break;

        switch (choice) {
            case 'v':
            case 'V':
                printf("Enter device command to verify: ");
                if (scanf("%63s", buffer) == 1) {
                    trim_whitespace(buffer);
                    verify_command(&reg, buffer);
                }
                break;

            case 'p':
            case 'P':
                if (reg.root == NULL) {
                    printf("[INFO] Registry is empty.\n");
                } else {
                    printf("\n--- APPROVED COMMANDS (ALPHABETICAL IN-ORDER) ---\n");
                    int serial = 1;
                    inorder_print(reg.root, &serial);
                    printf("-------------------------------------------------\n");
                }
                break;

            case 's':
            case 'S':
                printf("[STATUS] Stored commands: %d / %d (Max capacity: %d)\n",
                       reg.count, MAX_COMMANDS, MAX_COMMANDS);
                break;

            case 'x':
            case 'X':
                clear_registry(&reg);
                break;

            case 'q':
            case 'Q':
                clear_registry(&reg);
                printf("[EXIT] Session terminated cleanly. Goodbye!\n");
                return 0;

            default:
                printf("[INVALID] Unknown option '%c'. Valid options: v, p, s, x, q.\n", choice);
                break;
        }
    }

    clear_registry(&reg);
    return 0;
}
