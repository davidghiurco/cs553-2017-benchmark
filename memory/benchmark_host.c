//
// Written by David Ghiurco.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <sys/time.h>


double work(size_t blk_size, int num_threads, void *thread_function, char *block, char *cp_block);

void *read_and_write_thread(void *param);

void *seq_write_access_thread(void *param);

void *random_write_access_thread(void *param);

// A utility function to swap to integers
void swap (long *a, long *b);

// A utility function to generate a random permutation of arr[]
void randomize (long *arr, long n);

// parameter struct
struct thread_sub_block {
    int block_number;
    int num_blocks;
    size_t blk_size;
    char *block;
    char *cp_block;
};


// 1.28 GB block --> allows for equal split of work for all threads and block sizes
// because is divisible by (8 * 80,000,000)

#define GIGABYTE_BLOCK 1280000000
#define NUM_EXPERIMENT_REPEATS 15


int main(int argc, char *argv[]) {
    /*
     * Usage:
     * $ benchmark_host <type> <block_size> <num_threads>
     * type: 'read_and_write' or 'seq_write_access' or 'random_write_access'
     * block_size: # of bytes
     * num_threads: 1, 2, 4, 8
     */

    if (argc != 4) {
        printf("Error: 3 parameters required\n");
        exit(1);
    }

    size_t blk_size = (size_t) atoi(argv[2]);
    int num_threads = atoi(argv[3]);

    // all experiments will need a gigabyte block, but only the memcpy experiment needs a second gigabyte block
    char *block = malloc(GIGABYTE_BLOCK);
    if (block == NULL) {
        printf("Out of memory!\n");
        exit(1);
    }
    char *cp_block;

    double sum = 0;
    if (strcmp(argv[1], "read_and_write") == 0) { ;
        cp_block = malloc(GIGABYTE_BLOCK);
        if (cp_block == NULL) {
            printf("Out of memory!\n");
            exit(1);
        }
        // repeat the benchmark NUM_EXPERIMENT_REPEATs times, and aggregate the runtime in microseconds
        for (int i = 0; i < NUM_EXPERIMENT_REPEATS; i++)
            sum += work(blk_size, num_threads, read_and_write_thread, block, cp_block);
    } else if (strcmp(argv[1], "seq_write_access") == 0) {
        cp_block = NULL;
        for (int i = 0; i < NUM_EXPERIMENT_REPEATS; i++)
            sum += work(blk_size, num_threads, seq_write_access_thread, block, cp_block);
    } else if (strcmp(argv[1], "random_write_access") == 0) {
        cp_block = NULL;
        for (int i = 0; i < NUM_EXPERIMENT_REPEATS; i++)
            sum += work(blk_size, num_threads, random_write_access_thread, block, cp_block);
    } else {
        printf("Usage error\n");
        exit(1);
    }
    // Divide the total aggregated runtime by the total number of experiments to get an average throughput
    // Note: For the latency experiments, throughput will be converted to latency through unit conversions
    double mbps = sum / NUM_EXPERIMENT_REPEATS;
    printf("MBps: %f\n", mbps);

    free(block);
    free(cp_block);

    exit(0);
}

/*
 * Spawns the specified threads and allocates resources for them depending on the type of experiment
 * Returns the throughput (in MBps) for this experiment
 */
double work(size_t blk_size, int num_threads, void *thread_function, char *block, char *cp_block) {
    // char *block;
    pthread_t thread[num_threads];
    struct thread_sub_block args[num_threads];

    struct timeval start;
    struct timeval end;

    gettimeofday(&start, NULL);
    for (int num = 0; num < num_threads; num++) {
        args[num].block_number = num;
        args[num].cp_block = cp_block;
        args[num].num_blocks = num_threads;
        args[num].blk_size = blk_size;
        args[num].block = block;

        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_create(&(thread[num]), &attr, thread_function, &args[num]);
    }

    // Wait for the threads to finish
    for (int num = 0; num < num_threads; num++) {
        pthread_join(thread[num], NULL);;
    }
    gettimeofday(&end, NULL);

    // calculate the elapsed time in microseconds
    long elapsed_time_us = (end.tv_sec - start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec;

    // Megabytes / second is equivalent to bytes / microsecond
    return (double) GIGABYTE_BLOCK / elapsed_time_us;
}

/*
 * This is the memcpy thread function. Each thread will execute this function.
 * Takes in the argument struct. Will memcpy corresponding regions of the 1 gigabyte block and cp_block data structures
 * 'by blocks of size 'sub_block_size' at a time
 */
void *read_and_write_thread(void *param) {
    struct thread_sub_block *arg = param;
    int block_number = arg->block_number;
    int num_blocks = arg->num_blocks;
    size_t sub_block_size = arg->blk_size;
    char *block = arg->block;
    char *cp_block = arg->cp_block;

    long thread_block_size = (long) ceil(GIGABYTE_BLOCK / num_blocks);
    long start_index = block_number * thread_block_size;
    long end_index = (block_number + 1) * thread_block_size;


    for (long i = start_index; i < end_index; i += sub_block_size) {
        memcpy(&cp_block[i], &block[i], sub_block_size);
    }
}

/*
 * This is the sequential write access (memset) function. Each thread will execution this function
 * Takes in the argument struct. (Note, that in this case, cp_block should be set to NULL by the calling function)
 * Will memset corresponding regions of the gigabyte block that was passed in by blocks of size 'sub_block-size at a time
 */
void *seq_write_access_thread(void *param) {
    // unpack the parameters
    struct thread_sub_block *arg = param;
    int block_number = arg->block_number;
    int num_blocks = arg->num_blocks;
    size_t blk_size = arg->blk_size;
    char *block = arg->block;

    // calculate the bounds of this thread
    long thread_workload_block_size = (long) ceil(GIGABYTE_BLOCK / num_blocks); // chunk of the big block given to each thread
    long start_index = block_number * thread_workload_block_size;
    long end_index = (block_number + 1) * thread_workload_block_size;

    // iterate over each block and perform the memset operation
    for (long i = start_index; i < end_index; i += blk_size) {
        memset(&block[i], 'a', blk_size);
    }
}

/*
 * This is the random write access (memset) function. It functions just like the sequqntial write access function
 * but instead of memsetting consecutive blocks one after the other, it memsets random blocks (using the list of randomized indices)
 * Note: With small block size and low concurrency, this will take significantly longer than its sequential counterpart
 */
void *random_write_access_thread(void *param) {
    // unpack the parameters
    struct thread_sub_block *arg = param;
    int block_number = arg->block_number;
    int num_blocks = arg->num_blocks;
    size_t blk_size = arg->blk_size;
    char *block = arg->block;

    // calculate the bounds of this thread
    long thread_block_size = (int) ceil(GIGABYTE_BLOCK / num_blocks);
    long start_index = block_number * thread_block_size;
    long end_index = (block_number + 1) * thread_block_size;

    // create an array of randomized indices in order to simulate random access
    long num_sub_blocks = (long) ceil((end_index - start_index) / blk_size);

    // TODO: Setup this array outside of the thread so that the overhead is not included in the time
    long *block_indices = malloc (num_sub_blocks * sizeof(long));
    for (long i = 0; i < num_sub_blocks; i++) {
        block_indices[i] = i;;
    }
    randomize(block_indices, num_sub_blocks);

    // iterate over each block using the randomized index array to simulate random access, and perform memset
    for (long b = 0; b < num_sub_blocks; b++) {
        // get the starting index of a random block
        long current_index = start_index + block_indices[b] * (int) blk_size;
        memset(&block[current_index], 'a', blk_size);
    }
    free(block_indices);

}

void randomize(long *arr, long n) {
    // seed the random number generator so that we produce the same access pattern on every run of this program
    srand(100);
    //srand ( (unsigned int) time (NULL));

    // Start from the last element and swap one by one. We don't
    // need to run for the first element that's why i > 0
    for (long i = n-1; i > 0; i--)
    {
        // Pick a random index from 0 to i
        long j = rand() % (i+1);

        // Swap arr[i] with the element at random index
        swap(&arr[i], &arr[j]);
    }
}

void swap(long *a, long *b) {
    long temp = *a;
    *a = *b;
    *b = temp;
}


