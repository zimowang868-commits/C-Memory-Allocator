/* implements getmem (malloc) and freemem (free) functions for memory system */

#include <assert.h>
#include "mem.h"
#include "mem_impl.h"

// the size of list if needs to expand
#define EXPAND_LIST_SIZE 8192

// initializes global variables
freeNode* freelist = NULL;
uintptr_t totalmalloc = 0;

// Returns a pointer to a new block of storage with at least the given
// size bytes of memory
// The pointer to the returned block is aligned on an 16-byte boundary
void* getmem(uintptr_t size);

// Split the given large block into two smaller blocks
// and returns the second block
freeNode* splitNode(freeNode* curr, uintptr_t sizeReq);

// Returns the next block that contains the given size
freeNode* getNextAvailableNode(uintptr_t sizeReq);

// Returns a new node with the given size, next node to
// point to, and the given address
freeNode* makeNewNode(uintptr_t totalSize, freeNode* next, uintptr_t addr);

// Inserts a new node with given size to the given addr
void insertNode(uintptr_t totalSize, uintptr_t addr);

// Asks the given size of memory from the underlying system
// and adds the allocated memory to freelist
void addMem(uintptr_t sizeReq);

// Returns the block of storage at location p to the pool of
// available free storage. The pointer value p must be one
// obtained as the result of a call to getmem.
// If the given block is physically located in memory adjacent to one or
// more other free blocks, then the free blocks involved should be
// combined into a single larger block
void freemem(void* p);

// Inserts a new node back to the freelist with the given address
// and size.
// If the block is physically located in memory adjacent to one or
// more other free blocks, then the free blocks involved should be
// combined into a single larger block
void insertFreelist(uintptr_t addr, uintptr_t size);

// Combines all adjacent blocks into one large block.
void combineFreeBlock();

// Returns 1 if the given two nodes are adjacent
// Returns 0 if not
int isAdjacent(freeNode* node1, freeNode* node2);

// Returns a pointer to a new block of storage with at least the given
// size bytes of memory
// The pointer to the returned block is aligned on an 16-byte boundary
// If the memory block is too large, split it into smaller blocks
void* getmem(uintptr_t size) {
    // check_heap();
    if (size <= 0) {
        return NULL;
    }
    if (size % 16 != 0) {
        size = (size / 16) * 16 + 16;
    }

    if (freelist == NULL) {
        addMem(size);
    }

    // first node has enough space
    if (freelist != NULL && freelist->size >= size) {
        uintptr_t blockAddr = (uintptr_t)freelist + NODESIZE;
        if (freelist->size >= size + NODESIZE + MINCHUNK) {
            freeNode* curr = freelist;
            freeNode* newNode = splitNode(curr, size);
            curr->size = size;
            curr->next = newNode;
            freelist = newNode;
        }
        return (void*) blockAddr;
    } else {
        // first node does not have enough space
        // and look for nodes after the first node
        freeNode* reqNode = getNextAvailableNode(size);
        if (reqNode == NULL) {
            addMem(size);
            reqNode = getNextAvailableNode(size);
        }
        if (reqNode == NULL) return NULL;
        uintptr_t blockAddr = (uintptr_t)(reqNode->next) + NODESIZE;
        if (reqNode->next->size >= size && reqNode->next->size >= size + NODESIZE + MINCHUNK) {
            freeNode* newNode = splitNode(reqNode->next, size);
            reqNode->next->size = size;
            reqNode->next->next = newNode;
        }
        reqNode->next = reqNode->next->next;
        return (void*) blockAddr;
    }
    return NULL;
}

// Split the given large block into two smaller blocks
// and returns the second block
freeNode* splitNode(freeNode* curr, uintptr_t sizeReq) {
    uintptr_t newNodeSize = curr->size - sizeReq - NODESIZE - 1;
    uintptr_t newNodeAddr = (uintptr_t)curr + NODESIZE + sizeReq + 1;
    freeNode* newNode = makeNewNode(newNodeSize, curr->next, newNodeAddr);
    return newNode;
}

// Returns the next block that contains the given size
freeNode* getNextAvailableNode(uintptr_t sizeReq) {
    freeNode* curr = freelist;
    while (curr->next != NULL) {
        if (curr->next->size >= sizeReq) {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

// Returns a new node with the given size, next node to
// point to, and the given address
freeNode* makeNewNode(uintptr_t totalSize, freeNode* next, uintptr_t addr) {
    freeNode* newNode = (freeNode*)addr;
    if (newNode == NULL) {
        return NULL;
    }
    newNode->next = next;
    newNode->size = totalSize;
    return newNode;
}

// Inserts a new node with given size to the given addr
void insertNode(uintptr_t totalSize, uintptr_t addr) {
    if (freelist == NULL) {
        freelist = makeNewNode(totalSize, NULL, addr);
    }
    if (addr < (uintptr_t)freelist) {
        freeNode* newNode = makeNewNode(totalSize, freelist, addr);
        freelist = newNode;
    } else {
        freeNode* curr = freelist;
        while (curr->next != NULL && (addr > (uintptr_t)(curr->next))) {
            curr = curr->next;
        }
        curr->next = makeNewNode(totalSize, curr->next, addr);
    }
}

// Asks the given size of memory from the underlying system
// and adds the allocated memory to freelist
void addMem(uintptr_t sizeReq) {
    uintptr_t totalSize = EXPAND_LIST_SIZE;
    if (sizeReq > totalSize) {
        totalSize = (sizeReq / 16) * 16 + 16;
    }
    uintptr_t newMem = (uintptr_t)malloc(totalSize + NODESIZE);
    if (newMem == NULL) {
        freelist = NULL;
    }

    totalmalloc = totalmalloc + totalSize + NODESIZE;

    if (freelist == NULL) {
        freelist = makeNewNode(totalSize, NULL, newMem);
    } else {
        insertNode(totalSize, newMem);
    }
}

// Returns the block of storage at location p to the pool of
// available free storage. The pointer value p must be one
// obtained as the result of a call to getmem.
// If the given block is physically located in memory adjacent to one or
// more other free blocks, then the free blocks involved should be
// combined into a single larger block
void freemem(void* p) {
    if (p == NULL) {
        return;
    }
    uintptr_t addr = (uintptr_t)p - NODESIZE;
    freeNode* nodeP = (freeNode*)(addr - NODESIZE);
    uintptr_t sizeP = nodeP->size;
    insertFreelist(addr, sizeP);
}

// Inserts a new node back to the freelist with the given address
// and size.
// If the block is physically located in memory adjacent to one or
// more other free blocks, then the free blocks involved should be
// combined into a single larger block
void insertFreelist(uintptr_t addr, uintptr_t size) {
    if (freelist == NULL) {
        freelist = makeNewNode(size, NULL, addr);
    }

    // inserted block is the first block
    if (addr < (uintptr_t)freelist) {
        freeNode* newNode = makeNewNode(size, freelist, addr);
        freelist = newNode;
        combineFreeBlock();
        return;
    }

    // inserted block is not the first block
    freeNode* curr = freelist;
    int added = 0;
    while (curr->next != NULL) {
        if (addr > (uintptr_t)curr && addr < (uintptr_t)(curr->next)) {
            freeNode* insertedNode = makeNewNode(size, curr->next, addr);
            curr->next = insertedNode;
            added++;
            break;
        }
        curr = curr->next;
    }
    if (added == 0) {
        curr->next = makeNewNode(size, NULL, addr);
    }
    combineFreeBlock();
}

// Combines all adjacent blocks into one large block.
void combineFreeBlock() {
    freeNode* curr = freelist;
    if (curr == NULL) {
        return;
    }
    while (curr->next != NULL) {
        if (isAdjacent(curr, curr->next)) {
            freeNode* temp = curr->next;
            curr->next = temp->next;
            curr->size = curr->size + NODESIZE + temp->size;
        } else {
            curr = curr->next;
        }
    }
}

// Returns 1 if the given two nodes are adjacent
// Returns 0 if not
int isAdjacent(freeNode* node1, freeNode* node2) {
    uintptr_t addr1 = (uintptr_t)node1;
    uintptr_t addr2 = (uintptr_t)node2;
    uintptr_t size1 = node1->size;
    uintptr_t size2 = node2->size;
    if (addr1 < addr2) {
        return (addr1 + NODESIZE + size1) == addr2;
    } else {
        return (addr2 + NODESIZE + size2) == addr1;
    }
}