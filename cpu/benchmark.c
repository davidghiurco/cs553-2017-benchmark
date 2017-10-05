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
int N = 1024;

#define NUM_EXPERIMENT_REPEATS 50

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
 * This benchmark is setup as a "pessimistic benchmark" (just like HPL is)
 * What this means is essentially, only the operations which are actually useful to the calculation
 * of the matrix multiplication are counted as operations performed by this program.
 * Index calculations, loop increments, etc, are considered metadata/overhead.
 *
 * Note that if the metadata calculations were included in the metric, this benchmark would become a
 * "optimistic" benchmark, which would take into account all arithmetic operations performed.
 * That's not really useful in the real world however, because people mostly care about the "useful"
 * number of operations performed.
 *
 * Also, if this metadata operations were to be included, the performance of this benchmark would far exceed
 * the performance of the HPL benchmark, which is a strong indicator that it is too optimistic.
 *
 * Therefore, The number of operations that we care about depends ONLY on N. Each entry in matrix C takes 2N operations
 * (N multiplications + N additions) and there are N x N entries
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

    int NUM_OPS = 2 * N * (N * N);

    int num_threads = atoi(argv[2]);

    // stores the total runtime for each experiment iteration in microseconds
    long aggregate_runtime_us = 0;

    if (strcmp(argv[1], "flops") == 0) {
        for (int i = 0; i < NUM_EXPERIMENT_REPEATS; i++) {
            aggregate_runtime_us += flops(num_threads);
        }
        double average = NUM_OPS * NUM_EXPERIMENT_REPEATS / (double) aggregate_runtime_us;
        printf("GFlops: %f\n", average);

    }
    else if (strcmp(argv[1], "iops") == 0) {
        for (int i = 0; i < NUM_EXPERIMENT_REPEATS; i++) {
            aggregate_runtime_us += iops(num_threads);
        }
        double average = NUM_OPS * NUM_EXPERIMENT_REPEATS / (double) aggregate_runtime_us;
        printf("GIops: %f\n", average);
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

    // calculate the elapsed time in microseconds and return
    return (long) (end.tv_sec-start.tv_sec)*1000000 + end.tv_usec-start.tv_usec;

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
    return (long) (end.tv_sec-start.tv_sec)*1000000 + end.tv_usec-start.tv_usec;;
}

/*
 * worker thread spawned by function flops()
 * takes in a parameter struct pointer containing the bounds of the matrix problem for the particular thread
 */
void *float_matrix_thread(void *param) {
    struct float_matrix_block *arg = param; // the structure that holds the parameters of the thread
    int thread_partition = N / arg->num_threads;

    for(int i = (arg->tid) * thread_partition; i < thread_partition * (arg->tid + 1); i++){
        for (int j = 0; j < N; j++){
            arg->C[i*N + j] = 0;
            for(int k = 0; k < N; k++){
                arg->C[i*N + j] += arg->A[i*N + k] * arg->B[k*N + j];
            }
        }
    }
    pthread_exit(0);
}

/*
 * worker thread spawned by function iops()
 * takes in a parameter struct pointer containing the bounds of the matrix problem for the particular thread
 */
void *int_matrix_thread(void *param) {
    struct int_matrix_block *arg = param; // the structure that holds the parameters of the thread
    int thread_partition = N / arg->num_threads;

    for(int i = (arg->tid) * thread_partition; i < thread_partition * (arg->tid + 1); i++){
        for (int j = 0; j < N; j++){
            arg->C[i*N + j] = 0;
            for(int k = 0; k < N; k++){
                arg->C[i*N + j] += arg->A[i*N + k] * arg->B[k*N + j];
            }
        }
    }
    pthread_exit(0);
}
