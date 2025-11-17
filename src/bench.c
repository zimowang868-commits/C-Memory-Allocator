/** bench.c is the benchmarking /test program for mem memory management */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include "mem.h"

/* print_stats is a helper function that prints performance
  information about the memory system.  Requires a clock_t start
  time, and prints elapsed time plus results from get_mem_stats()
 */
void print_stats(clock_t start);

/* fill_mem is a helper function that takes in an address and the
  size of the memory allocation at that address.  It should fill
  the first 16 bytes (or up to size) with hexedecimal values.*/
void fill_mem(void *ptr, uintptr_t size);

// Prints the following to stdout:
// Total CPU time used by the benchmark test so far in seconds
// Total amount of storage acquired from the underlying system
// by the memory manager during the test so far
// Total number of blocks on the free storage list at this
// point in the test.
// Average number of bytes in the free storage blocks at this
// point in the test.
void printTrails(clock_t start, int ntrials, int NTRIALS);

/* print_stats is a helper function that prints performance
  information about the memory system.  Requires a clock_t start
  time, and prints elapsed time plus results from get_mem_stats()
*/
void print_stats(clock_t start) {
    uintptr_t tsize;
    uintptr_t tfree;
    uintptr_t tblocks;
    get_mem_stats(&tsize, &tfree, &tblocks);
    printf("Total Amount of Storage: %lu bytes\n", tsize);
    printf("Total Number of free blocks: %lu \n", tblocks);
    if (tblocks == 0) {
        printf("Average size of the free blocks: 0 bytes\n");
    } else {
        printf("Average size of the free blocks: %lu bytes\n", tfree / tblocks);
    }
}

/* fill_mem is a helper function that takes in an address and the
  size of the memory allocation at that address.  It should fill
  the first 16 bytes (or up to size) with hexedecimal values.*/
void fill_mem(void *ptr, uintptr_t size) {
    if (size > 16)
        size = 16;

    for (int i = 0; i < size; i++) {
        unsigned char *m = (unsigned char *)((uintptr_t)ptr + i);
        m = 0xFE;
    }
}

/* Synopsis:   bench (main)
  [ntrials] (10000) getmem + freemem calls
  [pctget] (50) % of calls that are get mem
  [pctlarge] (10) % of calls requesting more memory than lower limit
  [small_limit] (200) largest size in bytes of small block
  [large_limit] (20000) largest size in byes of large block
  [random_seed] (time) initial seed for randn
*/
int main(int argc, char **argv) {
  // Initialize the parameters
    int NTRIALS;
    int PCTGET;
    int PCTLARGE;
    int SMALL_L;
    int LARGE_L;

    (argc > 1) ? (NTRIALS = atoi(argv[1])) : (NTRIALS = 10000);
    (argc > 2) ? (PCTGET = atoi(argv[2])) : (PCTGET = 50);
    (argc > 3) ? (PCTLARGE = atoi(argv[3])) : (PCTLARGE = 10);
    (argc > 4) ? (SMALL_L = atoi(argv[4])) : (SMALL_L = 200);
    (argc > 5) ? (LARGE_L = atoi(argv[5])) : (LARGE_L = 20000);

    // initialize random number gen.
    (argc > 6) ? srand(atoi(argv[6])) : srand(time(NULL));

    printf("Running bench for %d trials, %d%% getmem calls.\n", NTRIALS, PCTGET);

    void *blocks[NTRIALS];
    int ntrials = 0, nblocks = 0;
    clock_t start = clock();

  // perform NTRIALS mem operations
    while (ntrials < NTRIALS) {
        ntrials++;
        // choose getmem
        if ((rand() % 100 + 1) <= PCTGET) {
            void *ptr;
            uintptr_t reqSize;
            // large block
            if ((rand() % 100 + 1) <= PCTLARGE) {
                reqSize = (uintptr_t)(rand() % (LARGE_L - SMALL_L) + SMALL_L + 1);
            } else {
                reqSize = (uintptr_t)(rand() % SMALL_L + 1);
            }
            ptr = getmem(reqSize);
            fill_mem(ptr, reqSize);
            blocks[nblocks] = ptr;
            nblocks++;
        } else {
            if (nblocks > 0) {
                int randBlock = rand() % nblocks;
                freemem(blocks[randBlock]);
                blocks[randBlock] = blocks[nblocks - 1];
                nblocks--;
            }
        }
        printTrails(start, ntrials, NTRIALS);
    }
    return EXIT_SUCCESS;
}

// Prints the following to stdout:
// Total CPU time used by the benchmark test so far in seconds
// Total amount of storage acquired from the underlying system
// by the memory manager during the test so far
// Total number of blocks on the free storage list at this
// point in the test.
// Average number of bytes in the free storage blocks at this
// point in the test.
void printTrails(clock_t start, int ntrials, int NTRIALS) {
    float tens = 0.1;
    if ((float)ntrials / NTRIALS >= tens || ntrials == NTRIALS - 1) {
        clock_t end = clock();
        double cpu_time = (double)(end - start) / CLOCKS_PER_SEC;
        printf("Total CPU time used by the bench test is %f seconds.\n", cpu_time);
        print_stats(start);
    }
}
