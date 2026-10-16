/**
 * ============================================================================
 * Task 5: Embedded Firmware Log Compression Utility
 * Data Structure: Huffman Tree & Priority Queue (Min-Heap / Sorted List)
 * 
 * Key Features:
 * - Reads device_log.txt and computes character frequency distribution
 * - Constructs optimal prefix-free binary tree via greedy Huffman algorithm
 * - Generates variable-length binary codes (shorter codes for frequent characters)
 * - Bit-level file compression saved to device_log.huff with complete reconstruction header
 * - Decompression engine rebuilding identical tree and restoring device_log_restored.txt
 * - Automatic byte-by-byte lossless verification comparing original vs restored
 * - Accurate reporting of file sizes and compression percentage
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CHARS 256
#define CODE_LEN 256

#define ORIGINAL_FILE "device_log.txt"
#define COMPRESSED_FILE "device_log.huff"
#define RESTORED_FILE "device_log_restored.txt"

/* Huffman Tree Node */
typedef struct HuffmanNode {
    unsigned char ch;
    int freq;
    struct HuffmanNode* left;
    struct HuffmanNode* right;
} HuffmanNode;

/* Bit stream writer for outputting bit-by-bit to file */
typedef struct {
    FILE* fp;
    unsigned char buffer;
    int bit_count;
} BitWriter;

void init_bit_writer(BitWriter* bw, FILE* fp) {
    bw->fp = fp;
    bw->buffer = 0;
    bw->bit_count = 0;
}

void write_bit(BitWriter* bw, int bit) {
    if (bit) {
        bw->buffer |= (1 << (7 - bw->bit_count));
    }
    bw->bit_count++;
    if (bw->bit_count == 8) {
        fputc(bw->buffer, bw->fp);
        bw->buffer = 0;
        bw->bit_count = 0;
    }
}

void flush_bits(BitWriter* bw) {
    if (bw->bit_count > 0) {
        fputc(bw->buffer, bw->fp);
        bw->buffer = 0;
        bw->bit_count = 0;
    }
}

/* Bit stream reader for reading bit-by-bit from file */
typedef struct {
    FILE* fp;
    unsigned char buffer;
    int bit_count;
} BitReader;

void init_bit_reader(BitReader* br, FILE* fp) {
    br->fp = fp;
    br->buffer = 0;
    br->bit_count = 0;
}

int read_bit(BitReader* br) {
    if (br->bit_count == 0) {
        int byte = fgetc(br->fp);
        if (byte == EOF) return -1;
        br->buffer = (unsigned char)byte;
        br->bit_count = 8;
    }
    int bit = (br->buffer >> (br->bit_count - 1)) & 1;
    br->bit_count--;
    return bit;
}

/* Allocate a new Huffman node */
HuffmanNode* create_node(unsigned char ch, int freq) {
    HuffmanNode* node = (HuffmanNode*)malloc(sizeof(HuffmanNode));
    if (node == NULL) {
        printf("[ERROR] Memory allocation failed for Huffman node!\n");
        return NULL;
    }
    node->ch = ch;
    node->freq = freq;
    node->left = NULL;
    node->right = NULL;
    return node;
}

/* Recursively free the Huffman tree */
void free_tree(HuffmanNode* root) {
    if (root == NULL) return;
    free_tree(root->left);
    free_tree(root->right);
    free(root);
}

/* Build Huffman Tree from an array of character frequencies */
HuffmanNode* build_huffman_tree(const int freq[], int* unique_count) {
    HuffmanNode* nodes[MAX_CHARS];
    int size = 0;

    /* Create initial leaf nodes for all characters that appear at least once */
    for (int i = 0; i < MAX_CHARS; i++) {
        if (freq[i] > 0) {
            nodes[size++] = create_node((unsigned char)i, freq[i]);
        }
    }

    *unique_count = size;
    if (size == 0) return NULL;

    /* Special case: only 1 unique character in file */
    if (size == 1) {
        HuffmanNode* root = create_node(0, nodes[0]->freq);
        root->left = nodes[0];
        return root;
    }

    /* Greedy loop: repeatedly combine the two nodes with the smallest frequencies */
    while (size > 1) {
        /* Find two smallest indices: min1 and min2 */
        int min1 = 0, min2 = 1;
        if (nodes[min1]->freq > nodes[min2]->freq) {
            min1 = 1; min2 = 0;
        }

        for (int i = 2; i < size; i++) {
            if (nodes[i]->freq < nodes[min1]->freq) {
                min2 = min1;
                min1 = i;
            } else if (nodes[i]->freq < nodes[min2]->freq) {
                min2 = i;
            }
        }

        HuffmanNode* left_child = nodes[min1];
        HuffmanNode* right_child = nodes[min2];

        /* Create internal parent node */
        HuffmanNode* parent = create_node(0, left_child->freq + right_child->freq);
        parent->left = left_child;
        parent->right = right_child;

        /* Remove the two smallest and insert the parent */
        int remove_higher = (min1 > min2) ? min1 : min2;
        int remove_lower = (min1 < min2) ? min1 : min2;

        nodes[remove_higher] = nodes[size - 1];
        nodes[remove_lower] = parent;
        size--;
    }

    return nodes[0]; /* Final remaining node is the root */
}

/* Generate prefix codes by traversing the tree (Left=0, Right=1) */
void generate_codes(HuffmanNode* root, char codes[MAX_CHARS][CODE_LEN], char* current_code, int depth) {
    if (root == NULL) return;

    /* If it is a leaf node, save its code */
    if (root->left == NULL && root->right == NULL) {
        current_code[depth] = '\0';
        strncpy(codes[root->ch], current_code, CODE_LEN - 1);
        codes[root->ch][CODE_LEN - 1] = '\0';
        return;
    }

    /* Traverse left with '0' */
    if (root->left != NULL) {
        current_code[depth] = '0';
        generate_codes(root->left, codes, current_code, depth + 1);
    }

    /* Traverse right with '1' */
    if (root->right != NULL) {
        current_code[depth] = '1';
        generate_codes(root->right, codes, current_code, depth + 1);
    }
}

/* Helper to get file size in bytes */
long get_file_size(const char* filename) {
    FILE* fp = fopen(filename, "rb");
    if (fp == NULL) return -1;
    fseek(fp, 0, SEEK_END);
    long sz = ftell(fp);
    fclose(fp);
    return sz;
}

/* Compress device_log.txt into device_log.huff */
void compress_log(void) {
    printf("\n--- STEP 1: READING & FREQUENCY ANALYSIS ---\n");
    FILE* in = fopen(ORIGINAL_FILE, "rb");
    if (in == NULL) {
        printf("[ERROR] Could not open '%s' for reading!\n", ORIGINAL_FILE);
        return;
    }

    int freq[MAX_CHARS] = {0};
    int total_chars = 0;
    int c;

    while ((c = fgetc(in)) != EOF) {
        freq[(unsigned char)c]++;
        total_chars++;
    }

    if (total_chars == 0) {
        printf("[WARNING] Input file is empty. Nothing to compress.\n");
        fclose(in);
        return;
    }

    printf("Total characters in log: %d\n\n", total_chars);
    printf("%-12s %-12s\n", "Character", "Frequency");
    printf("------------------------\n");
    for (int i = 0; i < MAX_CHARS; i++) {
        if (freq[i] > 0) {
            if (i == '\n') printf("%-12s %-12d\n", "'\\n'", freq[i]);
            else if (i == '\r') printf("%-12s %-12d\n", "'\\r'", freq[i]);
            else if (i == ' ') printf("%-12s %-12d\n", "'SPACE'", freq[i]);
            else printf("'%c'         %-12d\n", i, freq[i]);
        }
    }

    /* Build tree and generate codes */
    printf("\n--- STEP 2: HUFFMAN TREE & CODE GENERATION ---\n");
    int unique_chars = 0;
    HuffmanNode* root = build_huffman_tree(freq, &unique_chars);

    char codes[MAX_CHARS][CODE_LEN];
    for (int i = 0; i < MAX_CHARS; i++) codes[i][0] = '\0';

    char current_code[CODE_LEN];
    generate_codes(root, codes, current_code, 0);

    printf("%-12s %-18s\n", "Character", "Huffman Code");
    printf("------------------------------\n");
    for (int i = 0; i < MAX_CHARS; i++) {
        if (freq[i] > 0) {
            if (i == '\n') printf("%-12s %-18s\n", "'\\n'", codes[i]);
            else if (i == '\r') printf("%-12s %-18s\n", "'\\r'", codes[i]);
            else if (i == ' ') printf("%-12s %-18s\n", "'SPACE'", codes[i]);
            else printf("'%c'         %-18s\n", i, codes[i]);
        }
    }

    /* Step 3: Write Header and Compressed Bitstream to device_log.huff */
    printf("\n--- STEP 3: COMPRESSING & PACKING BITS ---\n");
    FILE* out = fopen(COMPRESSED_FILE, "wb");
    if (out == NULL) {
        printf("[ERROR] Could not create compressed file '%s'!\n", COMPRESSED_FILE);
        fclose(in);
        free_tree(root);
        return;
    }

    /* Write reconstruction header:
     * 1. Total character count (int)
     * 2. Number of unique characters (int)
     * 3. (ch, freq) pairs for each unique character
     */
    fwrite(&total_chars, sizeof(int), 1, out);
    fwrite(&unique_chars, sizeof(int), 1, out);

    for (int i = 0; i < MAX_CHARS; i++) {
        if (freq[i] > 0) {
            unsigned char ch = (unsigned char)i;
            fputc(ch, out);
            fwrite(&freq[i], sizeof(int), 1, out);
        }
    }

    /* Rewind input file to encode characters */
    fseek(in, 0, SEEK_SET);

    BitWriter bw;
    init_bit_writer(&bw, out);

    while ((c = fgetc(in)) != EOF) {
        const char* code = codes[(unsigned char)c];
        for (int i = 0; code[i] != '\0'; i++) {
            write_bit(&bw, (code[i] == '1') ? 1 : 0);
        }
    }
    flush_bits(&bw);

    fclose(in);
    fclose(out);
    free_tree(root);

    /* Step 4: Display Statistics */
    long orig_size = get_file_size(ORIGINAL_FILE);
    long comp_size = get_file_size(COMPRESSED_FILE);
    double ratio = (1.0 - ((double)comp_size / (double)orig_size)) * 100.0;

    printf("\n=======================================================\n");
    printf("             COMPRESSION STATISTICS                    \n");
    printf("=======================================================\n");
    printf(" Original size:     %ld bytes\n", orig_size);
    printf(" Compressed size:   %ld bytes\n", comp_size);
    printf(" Compression ratio: %.2f%% space saved\n", ratio);
    printf(" Output File:       %s\n", COMPRESSED_FILE);
    printf("=======================================================\n");
}

/* Decompress device_log.huff into device_log_restored.txt */
void decompress_log(void) {
    printf("\n--- DECOMPRESSION PROCEDURE ---\n");
    FILE* in = fopen(COMPRESSED_FILE, "rb");
    if (in == NULL) {
        printf("[ERROR] Compressed file '%s' not found! Please compress first.\n", COMPRESSED_FILE);
        return;
    }

    /* 1. Read reconstruction header */
    int total_chars = 0;
    int unique_chars = 0;

    if (fread(&total_chars, sizeof(int), 1, in) != 1 ||
        fread(&unique_chars, sizeof(int), 1, in) != 1) {
        printf("[ERROR] Corrupted compressed file header!\n");
        fclose(in);
        return;
    }

    int freq[MAX_CHARS] = {0};
    for (int i = 0; i < unique_chars; i++) {
        int ch = fgetc(in);
        int f = 0;
        if (ch == EOF || fread(&f, sizeof(int), 1, in) != 1) {
            printf("[ERROR] Failed to read frequency table from header!\n");
            fclose(in);
            return;
        }
        freq[(unsigned char)ch] = f;
    }

    /* 2. Reconstruct the identical Huffman Tree */
    int rebuilt_unique = 0;
    HuffmanNode* root = build_huffman_tree(freq, &rebuilt_unique);
    if (root == NULL) {
        printf("[ERROR] Failed to reconstruct Huffman tree!\n");
        fclose(in);
        return;
    }

    /* 3. Decode bit stream and write restored characters */
    FILE* out = fopen(RESTORED_FILE, "wb");
    if (out == NULL) {
        printf("[ERROR] Could not create restored file '%s'!\n", RESTORED_FILE);
        fclose(in);
        free_tree(root);
        return;
    }

    BitReader br;
    init_bit_reader(&br, in);

    HuffmanNode* current = root;
    int decoded_count = 0;

    while (decoded_count < total_chars) {
        /* If tree has only 1 unique character */
        if (current->left == NULL && current->right == NULL) {
            fputc(current->ch, out);
            decoded_count++;
            current = root;
            continue;
        }

        int bit = read_bit(&br);
        if (bit == -1) break; /* Unexpected EOF */

        if (bit == 0) {
            current = current->left;
        } else {
            current = current->right;
        }

        /* Check if we reached a leaf */
        if (current->left == NULL && current->right == NULL) {
            fputc(current->ch, out);
            decoded_count++;
            current = root; /* Jump back to root for next character */
        }
    }

    fclose(in);
    fclose(out);
    free_tree(root);

    printf("[SUCCESS] Decompression completed successfully.\n");
    printf(" Decoded %d characters into '%s'.\n", decoded_count, RESTORED_FILE);
}

/* Compare original and restored files byte-by-byte */
void verify_restored_log(void) {
    printf("\n--- LOSSLESS VERIFICATION CHECK ---\n");
    FILE* f1 = fopen(ORIGINAL_FILE, "rb");
    FILE* f2 = fopen(RESTORED_FILE, "rb");

    if (f1 == NULL) {
        printf("[ERROR] Original file '%s' missing!\n", ORIGINAL_FILE);
        if (f2) fclose(f2);
        return;
    }
    if (f2 == NULL) {
        printf("[ERROR] Restored file '%s' missing! Please decompress first.\n", RESTORED_FILE);
        fclose(f1);
        return;
    }

    int c1, c2;
    long byte_index = 0;
    int is_identical = 1;

    while (1) {
        c1 = fgetc(f1);
        c2 = fgetc(f2);

        if (c1 != c2) {
            is_identical = 0;
            break;
        }
        if (c1 == EOF && c2 == EOF) {
            break; /* Both reached EOF identically */
        }
        byte_index++;
    }

    fclose(f1);
    fclose(f2);

    printf("\n=======================================================\n");
    if (is_identical) {
        printf(" Verification: SUCCESS\n");
        printf(" The restored file is 100%% identical to the original.\n");
        printf(" Total Verified: %ld bytes (Lossless guarantee upheld)\n", byte_index);
    } else {
        printf(" Verification: FAILED\n");
        printf(" Difference detected at byte offset %ld (Original: 0x%02X, Restored: 0x%02X)\n",
               byte_index, (unsigned char)c1, (unsigned char)c2);
    }
    printf("=======================================================\n");
}

int main(void) {
    int option;

    printf("====================================================\n");
    printf("   EMBEDDED FIRMWARE LOG COMPRESSION (HUFFMAN)      \n");
    printf("====================================================\n");

    while (1) {
        printf("\nOperations:\n");
        printf("  1. Compress log (%s -> %s)\n", ORIGINAL_FILE, COMPRESSED_FILE);
        printf("  2. Decompress log (%s -> %s)\n", COMPRESSED_FILE, RESTORED_FILE);
        printf("  3. Verify restored log (%s == %s)\n", ORIGINAL_FILE, RESTORED_FILE);
        printf("  4. Exit\n");
        printf("Select option (1-4): ");

        if (scanf("%d", &option) != 1) {
            printf("[INVALID] Please enter a valid number (1-4).\n");
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            continue;
        }

        switch (option) {
            case 1:
                compress_log();
                break;
            case 2:
                decompress_log();
                break;
            case 3:
                verify_restored_log();
                break;
            case 4:
                printf("[EXIT] Exiting Compression Utility. Goodbye!\n");
                return 0;
            default:
                printf("[INVALID] Option %d not recognized. Choose 1 to 4.\n", option);
                break;
        }
    }

    return 0;
}
