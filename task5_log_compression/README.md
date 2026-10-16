# Task 5: Embedded Firmware Log Compression Utility

## 1. Problem Overview
Embedded devices transmitting telemetric logs across low-bandwidth radio/cellular links need to minimize data size before transmission. Because system logs contain vital diagnostic parameters (temperature, voltage, network status), compression must be **strictly lossless**—no byte can be modified or lost.

This project implements **Huffman Coding**, an optimal prefix-free variable-length entropy encoding algorithm in C.

---

## 2. Algorithm & Workflow

```
[ device_log.txt ]
        |
        v
1. Character Frequency Analysis  ---> Counts occurrences of all 256 byte values
        |
        v
2. Huffman Tree Construction      ---> Repeatedly merges lowest two frequency nodes
        |
        v
3. Code Generation (Prefix Free)  ---> Left branch = 0, Right branch = 1
        |
        v
4. Bit-Level Encoding             ---> Pack variable-length bits into bytes -> device_log.huff
        |
        v
5. Tree Rebuilding & Decode       ---> Read header, restore exact tree -> device_log_restored.txt
        |
        v
6. Lossless Verification Check    ---> Byte-by-byte comparison asserts 100% identical match
```

---

## 3. Key Implementation Details

* **File Header Serialization:** To decompress without external knowledge, `device_log.huff` includes a compact header:
  - Total original character count (`total_chars`)
  - Number of unique characters (`unique_chars`)
  - Table of `(character, frequency)` pairs
  The decompressor reconstructs the exact same tree deterministically from this header.
* **Bit Buffer Packing:** Because Huffman codes have arbitrary bit lengths (e.g., 2 bits, 5 bits), `BitWriter` accumulates bits in an 8-bit `unsigned char buffer` and flushes complete bytes to disk.
* **Exact Termination:** Reading stops when exactly `total_chars` have been restored, eliminating trailing padding bit corruption.
* **Verification Engine:** Compares `device_log.txt` and `device_log_restored.txt` byte-by-byte via `fgetc()`, confirming zero data loss.

---

## 4. Time & Space Complexity

| Operation | Time Complexity | Space Complexity |
| :--- | :---: | :---: |
| Frequency Counting | $O(N)$ | $O(K)$ where $K = 256$ |
| Tree Construction | $O(K \log K)$ or $O(K^2)$ | $O(K)$ nodes |
| Code Generation | $O(K)$ | $O(K \cdot L)$ code table |
| Encoding & Bit-Writing | $O(N)$ | $O(1)$ stream buffer |
| Decoding | $O(N \cdot L_{avg})$ | $O(K)$ tree memory |
| Verification | $O(N)$ | $O(1)$ stream buffer |

*(Where $N$ is the number of characters in the file, $K \le 256$ unique bytes, $L_{avg}$ average code length).*

---

## 5. Compilation & Execution

### Using MSVC (`cl.exe`):
```cmd
cl /W4 /MD main.c /Fe:log_compression.exe
log_compression.exe
```

### Using GCC (MinGW):
```cmd
gcc -Wall -Wextra -std=c99 main.c -o log_compression.exe
./log_compression.exe
```

---

## 6. Standardized Oral Defense (Q&A)

**Q1: Why is Huffman Coding called a "prefix-free" code?**  
*Answer:* In a Huffman Tree, every character is situated exclusively at an external **leaf node**. Because no leaf can have children, no character's binary path can ever be a prefix of another character's path. This enables instantaneous decoding without delimiters.

**Q2: How does the decompression program know how to decode without saving the tree explicitly?**  
*Answer:* We store the frequency table of unique characters in the header of `device_log.huff`. Because the greedy Huffman algorithm is deterministic, passing the saved frequency table through `build_huffman_tree` reproduces the identical tree structure and bit mappings.

**Q3: How do you avoid reading extra false characters from the padding bits in the last compressed byte?**  
*Answer:* We record the exact `total_chars` in the header. The decompression loop stops as soon as `decoded_count == total_chars`, ignoring any trailing zero bits in the final byte.
