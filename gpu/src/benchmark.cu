#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* number of threads defined in a block */
#define NUMTHREADS 64

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

/* function that initializes the values in a matrix given as paramenter,
 * and that has a definition and implementation dependent on the
 * definition of several macros in order to determine the data type of 
 * the matrix;
 */
__host__ void init(DTYPE *mat, int N)
{
    int i, j, sign;
    DTYPE x, y;

    srand(time(NULL));

    for (i = 0; i < N; ++i) {
        for (j = 0; j < N; ++j) {
            x = rand();
            y = rand() + 1;
            sign = (-1) * (rand() % 2 + 1);
            mat[i * N + j] = sign * (x / y);
        }
    }
}

/* function that prints the contents of a matrix given as parameter,
 * and that has a definition and implementation dependent on the
 * definition of several macros in order to determine the data type of
 * the matrix;
 */
__host__ void print_mat(DTYPE *mat, int N)
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
#ifdef DOUBLE
            printf("%lf ", mat[i * N + j]);
#elif FLOAT
            printf("%f ", mat[i * N + j]);
#endif
        }
        printf("\n");
    }
}

/* GPU device function that executes multiplication */
__global__ void multiply(DTYPE *A, DTYPE *B, DTYPE *C, int N) {
    int i, row, col, index;

    index = blockIdx.x * blockDim.x + threadIdx.x;
    row = index / N;
    col = index % N;

    C[index] = 0.0;
    for (i = 0; i < N; ++i) {
        C[index] += A[row * N + i] * B[i * N + col];
    }
}

int main(int argc, char **argv)
{
    int N;
    DTYPE *A, *B, *C;
    DTYPE *dA, *dB, *dC;
    clock_t start, end;

    if (argc <= 1 || argc >= 3) {
        perror("program usage: <./benchmark.exe> <size>");
        return -1;
    } else {
        N = atoi(argv[1]);
    }

    A = (DTYPE *) malloc(DSIZE * N * N);
    B = (DTYPE *) malloc(DSIZE * N * N);
    C = (DTYPE *) malloc(DSIZE * N * N);

    cudaMalloc((void **) &dA, DSIZE * N * N);
    cudaMalloc((void **) &dB, DSIZE * N * N);
    cudaMalloc((void **) &dC, DSIZE * N * N);

    init(A, N);
    init(B, N);

    if (DEBUG) {
        printf("A = \n");
        print_mat(A, N);
        printf("B = \n");
        print_mat(B, N);
    }

    cudaMemcpy(dA, A, DSIZE * N * N, cudaMemcpyHostToDevice);
    cudaMemcpy(dB, B, DSIZE * N * N, cudaMemcpyHostToDevice);
    
    start = clock();
    multiply<<<(N * N / NUMTHREADS), NUMTHREADS>>>(dA, dB, dC, N);
    cudaDeviceSynchronize();
    end = clock();

    cudaMemcpy(C, dC, DSIZE * N * N, cudaMemcpyDeviceToHost);
    
    if (DEBUG) {
        printf("C = \n");
        print_mat(C, N);
    }

    printf("Execution time: %ldus\n", 
            ((long) (end - start) * 1000000) / CLOCKS_PER_SEC);

    cudaFree(dA);
    cudaFree(dB);
    cudaFree(dC);

    free(A);
    free(B);
    free(C);

    return 0;
}
