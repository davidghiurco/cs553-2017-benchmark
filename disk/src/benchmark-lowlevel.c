#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#define SIZE8B 0
#define SIZE8KB 1
#define SIZE8MB 2
#define SIZE80MB 3

#define READWRITE 0
#define SEQUENTIAL 1
#define RANDOM 2

#define SETSIZE 10 * 128 * 1024 * 1024
#define DEBUG 1

typedef struct thread_arg_t
{
    int mode;
    int fd_in;
    int fd_out;
    long *pos_vec;
    int pos_start;
    int pos_length;
    int block_size;
    long runtime_total;
    long runtime_average;
} thread_arg_t;

void shuffle(long *vec, int n)
{
    int i, j;
    long temp;

    for (i = 0; i < n; ++i) {
        j = rand() % n;
        temp = vec[i];
        vec[i] = vec[j];
        vec[j] = temp;
    }
}

void init_pos(long *vec, int n, int block_size)
{
    int i;

    for (i = 0; i < n; ++i) {
        vec[i] = (long) i * (long) block_size;
    }
}

void *work(void *argv)
{
    thread_arg_t *arg = (thread_arg_t *) argv;
    char *buffer;
    int i;
    long rc, rd;
    clock_t start, end;

    buffer = (char *) malloc(arg->block_size * sizeof(char));

    for (i = 0; i < arg->pos_length; ++i) {
        start = clock();
        rc = 0;
        do {
            rd = pread(arg->fd_in, &buffer[rc], arg->block_size - rc,
                    arg->pos_vec[arg->pos_start + i] + rc);
            if (rd < 0) {
                printf("Error reading from file\n");
                if (DEBUG) {
                    printf("Error at position: %ld\n", 
                            arg->pos_vec[arg->pos_start + i] + rc);
                    printf("Block index: %d\n", i);
                    printf("Block start: %ld\n", 
                            arg->pos_vec[arg->pos_start + i]);
                    printf("Number of blocks: %d\n", arg->pos_length);
                    printf("Number of bytes read: %ld", rc);
                }
                free(buffer);
                pthread_exit(NULL);
            }
            rc += rd;
        } while (rc < arg->block_size);
        
        if (arg->mode == READWRITE) {
            rc = 0;
            do {
                rd = pwrite(arg->fd_out, &buffer[rc], arg->block_size - rc,
                        arg->pos_vec[arg->pos_start + i] + rc);
                if (rd < 0) {
                    printf("Error writing to file\n");
                    if (DEBUG) {
                        printf("Error at position %ld\n", 
                                arg->pos_vec[arg->pos_start + i] + rc);
                    }
                    free(buffer);
                    pthread_exit(NULL);
                }
                rc += rd;
            } while (rc < arg->block_size);
        }
        end = clock();
        arg->runtime_total += ((long) (end - start) * 1000000) / CLOCKS_PER_SEC;
    }

    arg->runtime_average = arg->runtime_total / arg->pos_length;

    free(buffer);

    pthread_exit(NULL);
}

int main(int argc, char **argv)
{
    pthread_t *threads;
    thread_arg_t *args;
    int fd_in, fd_out, rc, i;
    long *pos_vec;
    int num_threads, block_size, num_blocks, mode;

    if (argc <= 3 || argc >= 5) {
        printf("program usage: ./benchmark-lowlevel.exe"
                "<num_threads> <block_size> <mode>\n"
                "<block_size> accepts the following values:\n"
                "\t 0 -> 8B block size\n"
                "\t 1 -> 8KB block size\n"
                "\t 2 -> 8MB block size\n"
                "\t 3 -> 80MB block size\n"
                "<mode> accepts the following values:\n"
                "\t 0 -> READ+WRITE operations\n"
                "\t 1 -> SEQUENTIAL read\n"
                "\t 2 -> RANDOM read\n");
        exit(-1);
    } else {
        num_threads = atoi(argv[1]);
        switch (atoi(argv[2])) {
            case SIZE8B:
                block_size = 8;
                num_blocks = SETSIZE / ((block_size / 8) * 1024 * 1024);
                break;
            case SIZE8KB:
                block_size = 8 * 1024;
                num_blocks = SETSIZE / (block_size / 8);
                break;
            case SIZE8MB:
                block_size = 8 * 1024 * 1024;
                num_blocks = SETSIZE / (block_size / 8);
                break;
            case SIZE80MB:
                block_size = 80 * 1024 * 1024;
                num_blocks = SETSIZE / (block_size / 8);
                break;
            default:
                printf("Unsupported value for block size\n");
                exit(-1);
        }
        num_blocks = SETSIZE / (block_size / 8);
        switch (atoi(argv[3])) {
            case READWRITE:
                mode = READWRITE;
                break;
            case SEQUENTIAL:
                mode = SEQUENTIAL;
                break;
            case RANDOM:
                mode = RANDOM;
                break;
            default:
                printf("Unsupported value for mode\n");
                exit(-1);
        }
    }

    // opening input and output files //
    fd_in = open("file.in", O_RDONLY);
    if (fd_in < 0) {
        printf("Could not open input file file.in\n");
        exit (-2);
    }

    if (mode == READWRITE) {
        fd_out = open("file.out", O_WRONLY);
        if (fd_out < 0) {
            printf("Could not open output file file.out\n");
            close(fd_in);
            exit(-2);
        }
    } else {
        fd_out = -1;
    }

    threads = (pthread_t *) malloc(num_threads * sizeof(pthread_t));
    args = (thread_arg_t *) malloc(num_threads * sizeof(thread_arg_t));

    // initializing position vector for sequential or random file access //
    pos_vec = (long *) malloc(num_blocks * sizeof(long));
    init_pos(pos_vec, num_blocks, block_size);
    if (mode == RANDOM) {
        shuffle(pos_vec, num_blocks);
    }

    for (i = 0; i < num_threads; ++i) {
        args[i].mode = mode;
        args[i].fd_in = dup(fd_in);
        if (mode == READWRITE) {
            args[i].fd_out = dup(fd_out);
        } else {
            args[i].fd_out = -1;
        }
        args[i].pos_vec = pos_vec;
        args[i].pos_start = i * (num_blocks / num_threads);
        args[i].pos_length = num_blocks / num_threads;
        args[i].block_size = block_size;
        args[i].runtime_average = 0;
        args[i].runtime_total = 0;
        rc = pthread_create(&threads[i], NULL, work, &args[i]);
        
        if (rc) {
            printf("Could not create thread %d!\n", i);
            exit(-3);
        }
    }

    for (i = 0; i < num_threads; ++i) {
        rc = pthread_join(threads[i], NULL);
        
        if (rc) {
            printf("Could not terminate thread %d!\n", i);
            exit(-3);
        }
        
        close(args[i].fd_in);
        if (mode == READWRITE) {
            close(args[i].fd_out);
        }
    }

    for (i = 0; i < num_threads; ++i) {
        printf("thread %d total runtime: %ld us\n", i, args[i].runtime_total);
        printf("thread %d average latency: %ld us\n", i, 
                args[i].runtime_average);
    }

    free(pos_vec);
    free(threads);
    free(args);

    close(fd_in);
    if (mode == READWRITE) {
        close(fd_out);
    }

    pthread_exit(NULL);
}
