//
// Created by david on 9/16/17.
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
double flops(int num_threads);
double iops(int num_threads);
void *float_matrix_thread(void *param);
void *int_matrix_thread(void *param);

// global mutex
pthread_mutex_t lock;

// global variable which will store the total number of instructions
long num_operations = 0;

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

    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return 1;
    }

    if (argv[3] != NULL) {
        N = atoi(argv[3]);
    }

    int n = atoi(argv[2]);
    double sum = 0;
    if (strcmp(argv[1], "flops") == 0) {
        for (int i = 0; i < NUM_EXPERIMENT_REPEATS; i++) {
            sum += flops(n);
        }
        double average = sum / NUM_EXPERIMENT_REPEATS;
        printf("GFlops: %f\n", average);

    }
    else if (strcmp(argv[1], "iops") == 0) {
        for (int i = 0; i < NUM_EXPERIMENT_REPEATS; i++) {
            sum += iops(n);
        }
        double average = sum / NUM_EXPERIMENT_REPEATS;
        printf("GIops: %f\n", average);
    }
    else {
        printf("Usage error\n");
        exit(1);
    }

}


double flops(int num_threads) {
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

    // calculate the elapsed time in microseconds
    long elapsed_time_us = (end.tv_sec-start.tv_sec)*1000000 + end.tv_usec-start.tv_usec;

    // divide flops by 1000 instead of a billion because we are dividing by microseconds instead of seconds
    double gflops = (double) num_operations / elapsed_time_us / 1000;
    // printf("GFlops: %lf\n", gflops);

    free(A);
    free(B);
    free(C);

    return gflops;
}

double iops(int num_threads) {
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

    // calculate the elapsed time in microseconds
    long elapsed_time_us = (end.tv_sec-start.tv_sec)*1000000 + end.tv_usec-start.tv_usec;

    // divide flops by 1000 instead of a billion because we are dividing by microseconds instead of seconds
    double giops = (double) num_operations / elapsed_time_us / 1000;
    // printf("GIops: %lf\n", iops);


    free(A);
    free(B);
    free(C);
    return giops;
}

void *float_matrix_thread(void *param) {
    struct float_matrix_block *arg = param; // the structure that holds the parameters of the thread
    int thread_partition = (int) N / arg->num_threads;

    long count = 0;
    for(int i = (arg->tid) * thread_partition; i < thread_partition * (arg->tid + 1); i++){
        // Counter will need to accumulate the operations performed in each loop variable increment too
        count+=5;  // 4 operations in loop plus this increment
        for (int j = 0; j < N; j++){
            // C[i][j] = 0
            arg->C[i*N + j] = 0;
            count += 4; // 1 op from loop, 2 from array index, 1 from this increment
            for(int k = 0; k < N; k++){
                // C[i][j] += A[i][k] * B[k][j]
                arg->C[i*N + j] += arg->A[i*N + k] * arg->B[k*N + j];
                // 1 op from loop, 6 from array index, 1 multiplication, 1 addition, this increment
                count+=10;
            }
        }
    }
    pthread_mutex_lock(&lock);
    num_operations+=count;
    pthread_mutex_unlock(&lock);
    pthread_exit(0);
}

void *int_matrix_thread(void *param) {
    struct int_matrix_block *arg = param; // the structure that holds the parameters of the thread
    int thread_partition = (int) N / arg->num_threads;

    long count = 0;
    for(int i = (arg->tid) * thread_partition; i < thread_partition * (arg->tid + 1); i++){
        // Counter will need to accumulate the operations performed in each loop variable increment too
        count+=5;  // 4 operations in loop plus this increment
        for (int j = 0; j < N; j++){
            // C[i][j] = 0
            arg->C[i*N + j] = 0;
            count += 4; // 1 op from loop, 2 from array index, 1 from this increment
            for(int k = 0; k < N; k++){
                // C[i][j] += A[i][k] * B[k][j]
                arg->C[i*N + j] += arg->A[i*N + k] * arg->B[k*N + j];
                // 1 op from loop, 6 from array index, 1 multiplication, 1 addition, this increment
                count+=10;
            }
        }
    }
    pthread_mutex_lock(&lock);
    num_operations+=count;
    pthread_mutex_unlock(&lock);
    pthread_exit(0);
}
