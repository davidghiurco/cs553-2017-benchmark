//
// Written by David Ghiurco
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>

// The dimension of the matrix
// default, but can be changed by command-line input
long N = 1024;

#define NUM_EXPERIMENT_REPEATS 100

// prototypes
long flops(int num_threads);
long iops(int num_threads);
void *float_matrix_thread(void *param);
void *int_matrix_thread(void *param);

// parameters of the float matrix thread
struct float_matrix_block {
    const double *A, *B;
    double *C;
    int tid;
    int num_threads;
};

// parameters of the int matrix thread
struct int_matrix_block {
    const int *A, *B;
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
     * N (optional, default 1024) dimension of the square matrix
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

    // 4 operations are performed on each pass of the thread over an array value
    // array is size N*N
    // The number of times the experiment is repeated gets multiplied by the total number of operations of 1 experiment
    long NUM_OPS = 4 * N * N * NUM_EXPERIMENT_REPEATS;

    int num_threads = atoi(argv[2]);

    // stores the total runtime for each experiment iteration in microseconds
    long aggregate_runtime_us = 0;

    if (strcmp(argv[1], "flops") == 0) {
        for (int i = 0; i < NUM_EXPERIMENT_REPEATS; i++) {
            aggregate_runtime_us += flops(num_threads);
        }

        // Convert microseconds to seconds
        double aggregate_runtime_s = (double) aggregate_runtime_us / 1000000;

        printf("%lf\n", aggregate_runtime_s);
        double gflops = (double) NUM_OPS / aggregate_runtime_s / 1000000000 ;
        printf("GFlops: %lf\n", gflops);

    }
    else if (strcmp(argv[1], "iops") == 0) {
        for (int i = 0; i < NUM_EXPERIMENT_REPEATS; i++) {
            aggregate_runtime_us += iops(num_threads);
        }

        // Convert microseconds to seconds

        double aggregate_runtime_s = (double) aggregate_runtime_us / 1000000;
        double giops = (double) NUM_OPS / aggregate_runtime_s / 1000000000;

        printf("%ld\n", NUM_OPS);
        printf("%f\n", aggregate_runtime_s);
        printf("GIops: %f\n", giops);
    }
    else {
        printf("Usage error\n");
        exit(1);
    }

}

/*
 * Perform matrix multiplication on an N x N matrix of double-precision floats with the given number of threads
 *
 * return: runtime in microseconds
 */
long flops(int num_threads) {
    double *A, *B, *C;
    A = malloc(N * N * sizeof(double));
    B = malloc(N * N * sizeof(double));
    C = malloc(N * N * sizeof(double));

    if (A == NULL || B == NULL || C == NULL) {
        fprintf(stderr, "Error: not enough memory\n");
        exit(1);
    }

    // initialize the matrix with double-precision floats
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i*N + j] =  ((double) rand())/((double)RAND_MAX);
            B[i*N + j] =  ((double) rand())/((double)RAND_MAX);
        }
    }

    // build the parameter data structure for each thread and start the threads
    pthread_t thread[num_threads];
    struct timeval start;
    struct timeval end;
    gettimeofday(&start, NULL);
    for (int num = 0; num < num_threads; num++) {
        struct float_matrix_block arg;
        arg.A = A;
        arg.B = B;
        arg.C = C;
        arg.tid = num;
        arg.num_threads = num_threads;

        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_create(&(thread[num]), &attr, float_matrix_thread, &arg);
    }
    for (int num = 0; num < num_threads; num++) {
        pthread_join(thread[num], NULL);;
    }
    gettimeofday(&end, NULL);

    free(A);
    free(B);
    free(C);

    return ((long) (end.tv_sec-start.tv_sec)*1000000 + (long) (end.tv_usec-start.tv_usec));

}

/*
 * Perform matrix multiplication on an N x N matrix of integers with the given number of threads
 *
 * return: runtime in microseconds
 */
long iops(int num_threads) {
    int *A, *B, *C;
    A = malloc(N * N * sizeof(int));
    B = malloc(N * N * sizeof(int));
    C = malloc(N * N * sizeof(int));

    if (A == NULL || B == NULL || C == NULL) {
        fprintf(stderr, "Error: not enough memory\n");
        exit(1);
    }


    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i*N + j] = rand() + 1;
            B[i*N + j] = rand() + 1;
        }
    }
    pthread_t thread[num_threads];

    struct timeval start;
    struct timeval end;
    gettimeofday(&start, NULL);
    for (int num = 0; num < num_threads; num++) {
        struct int_matrix_block arg;
        arg.A = A;
        arg.B = B;
        arg.C = C;
        arg.tid = num;
        arg.num_threads = num_threads;

        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_create(&(thread[num]), &attr, int_matrix_thread, &arg);
    }
    for (int num = 0; num < num_threads; num++) {
        pthread_join(thread[num], NULL);;
    }
    gettimeofday(&end, NULL);

    free(A);
    free(B);
    free(C);

    // calculate the elapsed time in microseconds and return
    return ((long) (end.tv_sec-start.tv_sec)*1000000 + (long) (end.tv_usec-start.tv_usec));
}

//void *float_matrix_thread(void *param) {
//    struct float_matrix_block *arg = param; // the structure that holds the parameters of the thread
//    long thread_partition = N / arg->num_threads;
//
//    for(long i = (arg->tid) * thread_partition; i < thread_partition * (arg->tid + 1); i++){
//        for (int j = 0; j < N; j++){
//            arg->C[i*N + j] = 0;
//            for(int k = 0; k < N; k++){
//                arg->C[i*N + j] += arg->A[i*N + k] * arg->B[k*N + j];
//            }
//        }
//    }
//    pthread_exit(0);
//}

void *float_matrix_thread(void *param) {
    struct float_matrix_block *arg = param; // the structure that holds the parameters of the thread
    long thread_partition = N * N / (long) arg->num_threads;

    for(long i = (arg->tid) * thread_partition; i < thread_partition * (arg->tid + 1); i++){
        arg->C[i] = arg->A[i] * arg->B[i] * (float)  i + arg->B[i];
    }
    pthread_exit(0);
}


//void *int_matrix_thread(void *param) {
//    struct int_matrix_block *arg = param; // the structure that holds the parameters of the thread
//    long thread_partition = N / arg->num_threads;
//
//    for(long i = (arg->tid) * thread_partition; i < thread_partition * (arg->tid + 1); i++){
//        for (int j = 0; j < N; j++){
//            arg->C[i*N + j] = 0;
//            for(int k = 0; k < N; k++){
//                arg->C[i*N + j] += arg->A[i*N + k] * arg->B[k*N + j];
//            }
//        }
//    }
//    pthread_exit(0);
//}

void *int_matrix_thread(void *param) {
    struct int_matrix_block *arg = param; // the structure that holds the parameters of the thread
    long thread_partition = N * N / (long) arg->num_threads;

    for(long i = (arg->tid) * thread_partition; i < thread_partition * (arg->tid + 1); i++){
        arg->C[i] = arg->A[i] * arg->B[i] * (int) i + arg->B[i];
    }
    pthread_exit(0);
}