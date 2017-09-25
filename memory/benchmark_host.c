//
// Created by david on 9/16/17.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>

// global mutex
pthread_mutex_t lock;

// prototypes
char *get_blk(size_t blk_size);

int read_and_write(size_t blk_size, int num_threads);
int *read_and_write_thread(int block_number);

int seq_read_access(size_t blk_size, int num_threads);
int *seq_read_access_thread(int block_number);

int random_read_access(size_t blk_size, int num_threads);
int *random_read_access_thread(int block_number);


int main(int argc, char *argv[]) {
    /*
     * Usage:
     * $ benchmark_host <type> <block_size> <num_threads>
     * type: 'memcpy' or 'seq_read_access' or 'random_read_access'
     * block_size: # of bytes
     * num_threads: 1, 2, 4, 8
     */
    if (argc != 3 && argc != 4) {
        printf("Error: 2 parameters required");
        exit(1);
    }

    if (pthread_mutex_init(&lock, NULL) != 0) {
        printf("\n mutex init failed\n");
        return 1;
    }


    // Seed the random number generator for deterministic-ish results
    srand(100);

    size_t blk_size = (size_t) atoi(argv[2]);
    int num_threads = atoi(argv[3]);
    if (strcmp(argv[1], "read_and_write") == 0) {
        exit(read_and_write(blk_size, num_threads));
    } else if (strcmp(argv[1], "seq_read_access") == 0) {
        exit(seq_read_access(blk_size, num_threads));
    } else if (strcmp(argv[1], "random_read_access") == 0) {
        exit(random_read_access(blk_size, num_threads));
    } else {
        printf("Usage error\n");
        exit(1);
    }
}

/*
 * Returns a block of initialized chars (randomized)
 */
char *get_blk(size_t blk_size) {
    char *blk = malloc(blk_size);
    for (size_t i = 0; i < blk_size; i++) {
        blk[i] = (char) ('a' + (rand() % 26));
    }
    return blk;
}


int read_and_write(size_t blk_size, int num_threads) {
    char *blocks[num_threads];
    char *cp_blocks[num_threads];
    pthread_t thread[num_threads];

    // allocate a block for each thread to work on
    for (int num = 0; num < num_threads; num++) {
        blocks[num] = get_blk(blk_size);
        cp_blocks[num] = malloc(blk_size);
    }


    for (int num = 0; num < num_threads; num++) {

    }
    for (int num = 0; num < num_threads; num++) {
        pthread_join(thread[num], NULL);;
    }

    // free the blocks
    for (int num = 0; num < num_threads; num++) {
        free(blocks[num]);
        free(cp_blocks[num]);
    }
}

int *read_and_write_thread(int block_number) {

}


int seq_read_access(size_t blk_size, int num_threads) {
    return 0;
}

int random_read_access(size_t blk_size, int num_threads) {
    return 0;
}


