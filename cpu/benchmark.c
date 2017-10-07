//
// Written by David Ghiurco
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>

// the length of the vector
// default, but can be changed by command-line input
long N = 512000; // 51,200

#define NUM_EXPERIMENT_REPEATS 100000

// prototypes
long flops(int num_threads);

long iops(int num_threads);

void *float_matrix_thread(void *param);

void *int_matrix_thread(void *param);

// parameters of the float vector thread
struct float_vector_block {
    double *C;
    int tid;
    int num_threads;
};

// parameters of the int matrix thread
struct int_vector_block {
    int *C;
    int tid;
    int num_threads;
};

/*
 * This benchmark performs modified vector multiplication
 */
int main(int argc, char *argv[]) {
    /*
     * Usage:
     * $ benchmark <type> <num_threads> <N>
     * type: 'flops' or 'iops'
     * num_threads: 1, 2, 4, 8
     */

    if (argc != 3 && argc != 4) {
        printf("Error: 2 parameters required");
        exit(1);
    }

    // Seed the random number generator for deterministic-ish results
    srand(50);

    // if N was provided as commandline parameter, use that instead of the top-level defined N dimension
    if (argv[3] != NULL) {
        N = atoi(argv[3]);
    }

    // each thread makes 2 operations over each element of the N-vector, NUM_EXPERIMENT_REPEATS times
    long NUM_OPS = 2 * N * NUM_EXPERIMENT_REPEATS;

    int num_threads = atoi(argv[2]);

    // stores the total runtime for each experiment iteration in microseconds
    long aggregate_runtime_us = 0;

    if (strcmp(argv[1], "flops") == 0) {
        aggregate_runtime_us += flops(num_threads);

        // Convert microseconds to seconds
        double aggregate_runtime_s = (double) aggregate_runtime_us / 1000000;
        double gflops = (double) NUM_OPS / aggregate_runtime_s / 1000000000;
        printf("GFlops: %lf\n", gflops);

    } else if (strcmp(argv[1], "iops") == 0) {
        aggregate_runtime_us += iops(num_threads);

        // Convert microseconds to seconds
        double aggregate_runtime_s = (double) aggregate_runtime_us / 1000000;
        double giops = (double) NUM_OPS / aggregate_runtime_s / 1000000000;
        printf("GIops: %f\n", giops);
    } else {
        printf("Usage error\n");
        exit(1);
    }

}

/*
 * Allocates needed resources for the vector multiplication problem (a vector of doubles),
 * spawns the specified number of threads
 * and waits for them to complete
 *
 * Returns the runtime in microseconds
 */
long flops(int num_threads) {
    double *C;
    C = malloc(N * sizeof(double));

    // initialize the vector with double-precision floats
    for (int i = 0; i < N; i++) {
        C[i] = ((double) rand()) / ((double) RAND_MAX);
    }

    // build the parameter data structure for each thread and start the threads
    pthread_t thread[num_threads];
    struct float_vector_block args[num_threads];

    struct timeval start;
    struct timeval end;
    gettimeofday(&start, NULL);
    for (int num = 0; num < num_threads; num++) {
        args[num].C = C;
        args[num].tid = num;
        args[num].num_threads = num_threads;

        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_create(&(thread[num]), &attr, float_matrix_thread, &args[num]);
    }
    for (int num = 0; num < num_threads; num++) {
        pthread_join(thread[num], NULL);;
    }
    gettimeofday(&end, NULL);

    free(C);

    return ((long) (end.tv_sec - start.tv_sec) * 1000000 + (long) (end.tv_usec - start.tv_usec));

}

/*
 * Allocates needed resources for the vector multiplication problem (a vector of integers),
 * spawns the specified number of threads
 * and waits for them to complete
 *
 * Returns the runtime in microseconds
 */
long iops(int num_threads) {
    int *C;
    C = malloc(N * sizeof(int));

    for (int i = 0; i < N; i++) {
        C[i] = rand() + 1;
    }

    pthread_t thread[num_threads];
    struct int_vector_block args[num_threads];


    struct timeval start;
    struct timeval end;
    gettimeofday(&start, NULL);
    for (int num = 0; num < num_threads; num++) {
        args[num].C = C;
        args[num].tid = num;
        args[num].num_threads = num_threads;

        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_create(&(thread[num]), &attr, int_matrix_thread, &args[num]);
    }
    for (int num = 0; num < num_threads; num++) {
        pthread_join(thread[num], NULL);;
    }
    gettimeofday(&end, NULL);

    free(C);

    // calculate the elapsed time in microseconds and return
    return ((long) (end.tv_sec - start.tv_sec) * 1000000 + (long) (end.tv_usec - start.tv_usec));
}

/*
 * The thread function for floating point operations
 * For each pass of the loop, 2 floating point operations are performed on the vector
 */
void *float_matrix_thread(void *param) {
    struct float_vector_block *arg = param; // the structure that holds the parameters of the thread
    long thread_partition = N / (long) arg->num_threads;

    for (int j = 0; j < NUM_EXPERIMENT_REPEATS; j++) {
        for (long i = (arg->tid) * thread_partition; i < thread_partition * (arg->tid + 1); i++) {
            arg->C[i] = arg->C[i] * arg->C[i] + arg->C[i];
        }
    }
    pthread_exit(0);
}


/*
 * The thread function for integer operations
 * For each pass of the loop, 2 floating point operationsare performed on the vector
 */
void *int_matrix_thread(void *param) {
    struct int_vector_block *arg = param; // the structure that holds the parameters of the thread
    long thread_partition = N / (long) arg->num_threads;

    long next_block = thread_partition * (arg->tid + 1);
    for (int j = 0; j < NUM_EXPERIMENT_REPEATS; j++) {
        for (long i = (arg->tid) * thread_partition; i < next_block; i++) {
            arg->C[i] = arg->C[i] * arg->C[i] + arg->C[i];
        }
    }
    pthread_exit(0);
}