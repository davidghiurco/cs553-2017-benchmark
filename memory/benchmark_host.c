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
void *read_and_write_thread(void *param);

int seq_read_access(size_t blk_size, int num_threads);
void *seq_read_access_thread(void *param);

int random_read_access(size_t blk_size, int num_threads);
void *random_read_access_thread(void *param);

// parameter struct
struct thread_sub_block {
    int block_number;
    size_t sub_block_size;
    char *block;
    char *cp_block;
};


int main(int argc, char *argv[]) {
    /*
     * Usage:
     * $ benchmark_host <type> <block_size> <num_threads>
     * type: 'memcpy' or 'seq_read_access' or 'random_read_access'
     * block_size: # of bytes
     * num_threads: 1, 2, 4, 8
     */

    if (argc != 4) {
        printf("Error: 3 parameters required\n");
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
    char *block;
    char *cp_block;
    pthread_t thread[num_threads];

    // allocate a block on the heap and initialize it
    block = get_blk(blk_size);
    // allocate memory for the copy block
    cp_block = malloc(blk_size);
    // each thread gets an equal partition of the overall block to work on
    size_t sub_block_size = blk_size / num_threads;

    struct timeval start;
    struct timeval end;

    gettimeofday(&start, NULL);
    for (int num = 0; num < num_threads; num++) {
        struct thread_sub_block arg;
        arg.block_number = num;
        arg.sub_block_size = sub_block_size;
        arg.block = block;
        arg.cp_block = cp_block;

        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_create(&(thread[num]), &attr, read_and_write_thread, &arg);
    }

    for (int num = 0; num < num_threads; num++) {
        pthread_join(thread[num], NULL);;
    }
    gettimeofday(&end, NULL);

    // calculate the elapsed time in microseconds
    long elapsed_time_us = (end.tv_sec-start.tv_sec)*1000000 + end.tv_usec-start.tv_usec;

    // Megabytes / second is equivalent to bytes / microsecond
    double mbps = (double) blk_size / elapsed_time_us;
    printf("Mbps: %f\n", mbps);

    free(block);
    free(cp_block);
}

void *read_and_write_thread(void *param) {
    struct thread_sub_block *arg = param;
    int block_number = arg->block_number;
    size_t sub_block_size = arg->sub_block_size;
    char *block = arg->block;
    char *cp_block = arg->cp_block;

    memcpy(&cp_block[block_number * sub_block_size], &block[block_number * sub_block_size], sub_block_size);
}


void *seq_read_access_thread(void *param) {

}

void *random_read_access_thread(void *param) {

}


