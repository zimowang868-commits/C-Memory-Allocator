# C Memory Allocator (Freelist-Based Dynamic Memory Manager)

This project implements a lightweight **dynamic memory allocator** in C, featuring a custom-designed **freelist** system for manual memory management.  
It recreates the core behaviors of `malloc()` and `free()` through two primary operations:

- **getmem()** — allocate a block of memory of at least a specified size  
- **freemem()** — free a block of memory and return it to the freelist  

The project demonstrates systems-level programming skills, pointer manipulation, memory layout reasoning, and performance-aware data structure design.

---

## 🔧 Overview

The allocator maintains a **freelist**, implemented as a sorted linked list of available memory blocks.  
Each node in the freelist contains:

- `size` — size of the free block  
- `next` — pointer to the next free block  

The allocator always keeps the freelist **sorted by memory address**, enabling efficient coalescing of adjacent free blocks.

---

## 🚀 Features

### ### ✨ `getmem(size)`
Allocates a memory block of at least `size` bytes:

1. Scans the freelist for the first block large enough to satisfy the request  
2. If the block is larger than needed, it is **split** into two blocks  
3. If no suitable block exists, the allocator requests more space from the underlying system  
4. Returns a pointer to the usable memory region

### ### ✨ `freemem(ptr)`
Returns a previously allocated block back to the freelist:

1. Inserts the block into the freelist in **sorted memory order**  
2. Checks whether the freed block is adjacent to surrounding nodes  
3. If adjacent, merges them into a **single large block** (coalescing) to avoid fragmentation

---

## 📊 Runtime Statistics (Optional Debug Output)

When enabled, the allocator prints the following diagnostic information:

- Total CPU time used by the benchmark  
- Total memory acquired from the operating system  
- Number of free blocks currently on the freelist  
- Average block size in the freelist  

These statistics help evaluate fragmentation, system calls, and performance characteristics.

---

## 🧪 Benchmark Summary

- Running 10,000 allocation/free cycles over 5 trials produced the following runtimes: 0.968s, 0.985s, 0.975s, 0.902s, 0.977s
- Estimated average runtime: 0.961 seconds per 10,000 operations
