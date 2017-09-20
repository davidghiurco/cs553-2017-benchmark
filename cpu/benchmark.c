//
// Created by david on 9/16/17.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

// The dimension of the matrix
#define N 500

int flops(int num_threads);
int iops(int num_threads);

void *float_thread(void *param);
void *int_thread(void *param);

pthread_mutex_t lock;

// global variable which will store the total number of instructions
// executed this run
// synchronized with a mutex
int counter = 0;

struct float_matrix_block {
    const double *A;
    const double *B;
    double *C;
    int tid;
    int num_threads;
};

int main(int argc, char *argv[]) {
    /*
     * Usage:
     * $ benchmark <type> <num_threads>
     * type: 'float' or 'int'
     * num_threads: 1, 2, 4, 8
     */

    if (argc != 3) {
        printf("Error: 2 parameters required");
        exit(1);
    }

    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return 1;
    }

    int n = atoi(argv[2]);
    if (strcmp(argv[1], "float") == 0) {
        exit(flops(n));
    }
    else if (strcmp(argv[1], "int") == 0) {
        exit(iops(n));
    }
    else {
        printf("Usage error\n");
    }

}


int flops(int num_threads) {
    double *A, *B, *C;
    A = malloc(N * N * sizeof(double));
    B = malloc(N * N * sizeof(double));
    C = malloc(N * N * sizeof(double));

    if (A == NULL || B == NULL || C == NULL) {
        fprintf(stderr, "Error: not enough memory\n");
        exit(1);
    }

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i*N + j] =  ((double) rand())/((double)RAND_MAX);
            B[i*N + j] =  ((double) rand())/((double)RAND_MAX);

        }
    }

    pthread_t thread[num_threads];


    clock_t start = clock(), diff;
    for (int num = 0; num < num_threads; num++) {
        struct float_matrix_block arg;
        arg.A = A;
        arg.B = B;
        arg.C = C;
        arg.tid = num;
        arg.num_threads = num_threads;

        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_create(&(thread[num]), &attr, float_thread, &arg);
    }

    for (int num = 0; num < num_threads; num++) {
        pthread_join(thread[num], NULL);
    }
    diff = clock() - start;

    double gigaflop = (double) counter / 1000000000;
    double time = diff / CLOCKS_PER_SEC;

    double gflops = gigaflop / time;
    printf("GFlops: %f\n", gflops);

    free(A);
    free(B);
    free(C);

}

void *float_thread(void *param) {
    struct float_matrix_block *arg = param; // the structure that holds the parameters of the thread
    int thread_partition = (int) N / arg->num_threads;

    for(int i = (arg->tid) * thread_partition; i < thread_partition * (arg->tid + 1); i++){
        // Counter will need to accumulate the operations performed in each loop variable increment too
        pthread_mutex_lock(&lock);
        counter+=5; // 4 operations in loop plus this increment
        pthread_mutex_unlock(&lock);
        for (int j = 0; j < N; j++){
            // C[i][j] = 0
            arg->C[i*N + j] = 0;
            pthread_mutex_lock(&lock);
            counter += 4; // 1 op from loop, 2 from array index, 1 from this increment
            pthread_mutex_unlock(&lock);
            for(int k = 0; k < N; k++){
                // C[i][j] += A[i][k] * B[k][j]
                arg->C[i*N + j] += arg->A[i*N + k] * arg->B[k*N + j];
                pthread_mutex_lock(&lock);
                // 1 op from loop, 6 from array index, 1 multiplication, 1 addition, this increment
                counter+=10;
                pthread_mutex_unlock(&lock);
            }
        }
    }
    pthread_exit(0);
}

int iops(int num_threads) {
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



    free(A);
    free(B);
    free(C);
}