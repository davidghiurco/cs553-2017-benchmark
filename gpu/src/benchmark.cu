#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* number of threads defined in a block */
//#define NUMTHREADS 64

/* size of the vectors */
#define DLEN 262144

/* debug mode prints the contents of the matrices after the calculation
 * 0 - deactivate debug mode
 * 1 - activate debug mode
 */
#define DEBUG 0

/* macro definition set up at compile time, deciding the data type
 * and precision to be used;
 */
#ifdef DOUBLE
#define DSIZE sizeof(double)
typedef double DTYPE;
#elif FLOAT
#define DSIZE sizeof(float)
typedef float DTYPE;
#endif

/* function that initializes the values in a vector given as paramenter,
 * and that has a definition and implementation dependent on the
 * definition of several macros in order to determine the data type of 
 * the vector;
 */
__host__ void init(DTYPE *vec, int N)
{
    int i, sign;
    DTYPE x, y;

    for (i = 0; i < N; ++i) {
        x = rand() % 100;
        y = (rand() + 1) % 100;
        sign = (-1) + (rand() % 2 * 2);
        vec[i] = sign * (x / y);
    }
}

/* function that prints the contents of a vector given as parameter,
 * and that has a definition and implementation dependent on the
 * definition of several macros in order to determine the data type of
 * the vector;
 */
__host__ void print_vec(DTYPE *vec, int N)
{
    int i;

    for (i = 0; i < N; i++) {
#ifdef DOUBLE
        printf("%lf ", vec[i]);
#elif FLOAT
        printf("%f ", vec[i]);
#endif
    }
    printf("\n");
}

/* GPU device function that D = A * B * scalar + C */
__global__ void func(DTYPE *A, DTYPE *B, DTYPE *C, DTYPE *D, DTYPE s, int N) {
    int index = blockIdx.x * blockDim.x + threadIdx.x;
    D[index] = A[index] * B[index] * s + C[index];
}

int main(int argc, char **argv)
{
    int N, i, NUMTHREADS;
    DTYPE *A, *B, *C, *D, scalar;
    DTYPE *dA, *dB, *dC, *dD;
    clock_t start, end;

    if (argc <= 1 || argc >= 4) {
        perror("program usage: <./benchmark.exe> <iterations> <num_threads");
        return -1;
    } else {
        N = atoi(argv[1]);
        NUMTHREADS = atoi(argv[2]);
    }

    A = (DTYPE *) malloc(DSIZE * DLEN);
    B = (DTYPE *) malloc(DSIZE * DLEN);
    C = (DTYPE *) malloc(DSIZE * DLEN);
    D = (DTYPE *) malloc(DSIZE * DLEN);

    cudaMalloc((void **) &dA, DSIZE * DLEN);
    cudaMalloc((void **) &dB, DSIZE * DLEN);
    cudaMalloc((void **) &dC, DSIZE * DLEN);
    cudaMalloc((void **) &dD, DSIZE * DLEN);

    srand(time(NULL));
    init(A, DLEN);
    init(B, DLEN);
    init(C, DLEN);
    scalar = ((-1) + (rand() % 2 * 2)) * (rand() % 10 + 1);

    if (DEBUG) {
        printf("A = \n");
        print_vec(A, DLEN);
        printf("B = \n");
        print_vec(B, DLEN);
        printf("C = \n");
        print_vec(C, DLEN);
#ifdef DOUBLE
        printf("scalar = %lf\n", scalar);
#elif FLOAT
        printf("scalar = %f\n", scalar);
#endif
    }

    cudaMemcpy(dA, A, DSIZE * DLEN, cudaMemcpyHostToDevice);
    cudaMemcpy(dB, B, DSIZE * DLEN, cudaMemcpyHostToDevice);
    cudaMemcpy(dC, C, DSIZE * DLEN, cudaMemcpyHostToDevice);
    
    start = clock();
    for (i = 0; i < N; ++i) {
        func<<<(DLEN / NUMTHREADS), NUMTHREADS>>>(dA, dB, dC, dD, scalar, DLEN);
        cudaDeviceSynchronize();
    }
    end = clock();

    cudaMemcpy(D, dD, DSIZE * DLEN, cudaMemcpyDeviceToHost);
    
    if (DEBUG) {
        printf("D = \n");
        print_vec(D, DLEN);
    }

    printf("Execution time: %ldus\n", 
            ((long) (end - start) * 1000000) / CLOCKS_PER_SEC);

    cudaFree(dA);
    cudaFree(dB);
    cudaFree(dC);
    cudaFree(dD);

    free(A);
    free(B);
    free(C);
    free(D);

    return 0;
}
