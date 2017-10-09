#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

#define MODE_LATENCY 0
#define MODE_THROUGHPUT 1

#define TYPE_CLIENT 0
#define TYPE_SERVER 1

#define PACKET_SIZE 1024
#define NUM_MESSAGES 64 * 8 * 1024
#define NUM_PACKETS 64 * 128 * 1024

typedef struct thread_arg_t
{
    struct sockaddr *srv;
    size_t addrlen;
    int sockfd;
    int mode;
    int num_messages;
    int num_packets;
    long runtime;
} thread_arg_t;

void init_dataset(char *dataset, int n)
{
    int i;

    for (i = 0; i < n; i++) {
        dataset[i] = rand() % 255 + 1;
    }
}

void *work_server(void *argv)
{
    thread_arg_t *arg;
    struct sockaddr_storage clt;
    socklen_t addrlen;
    char *buffer;
    int newfd, rc, rd, i;

    arg = (thread_arg_t *) argv;

    if (listen(arg->sockfd, 10) < 0) {
        fprintf(stderr, "Could not listen on socket!\n");
        pthread_exit(NULL);
    }

    newfd = accept(arg->sockfd, (struct sockaddr *) &clt, &addrlen);

    if (newfd < 0) {
        fprintf(stderr, "Could not accept socket!\n");
        pthread_exit(NULL);
    }

    buffer = (char *) malloc(PACKET_SIZE * sizeof(char));

    if (arg->mode == MODE_LATENCY) {
        for (i = 0; i < arg->num_messages; ++i) {
            memset(buffer, 0, PACKET_SIZE);
            
            rc = 0;
            while (rc < PACKET_SIZE) {
                rd = recv(newfd, &buffer[rc], PACKET_SIZE - rc, 0);
                
                if (rd == 0) {
                    fprintf(stdout, "Connection closed at receive!\n");
                    break;
                }

                if (rd < 0) {
                    fprintf(stderr, "Could not receive package!\n");
                    free(buffer);
                    pthread_exit(NULL);
                }
                
                rc += rd;
            }

            rc = 0;
            while (rc < PACKET_SIZE) {
                rd = send(newfd, &buffer[rc], PACKET_SIZE - rc, 0);

                if (rd < 0) {
                    fprintf(stderr, "Could not send package!\n");
                    free(buffer);
                    pthread_exit(NULL);
                }
                
                rc += rd;
            }
        }
    } else {
        for (i = 0; i < arg->num_packets; ++i) {
            memset(buffer, 0, PACKET_SIZE);
            
            rc = 0;
            while (rc < PACKET_SIZE) {
                rd = recv(newfd, &buffer[rc], PACKET_SIZE - rc, 0);
                
                if (rd == 0) {
                    fprintf(stdout, "Connection closed at receive!\n");
                    break;
                }

                if (rd < 0) {
                    fprintf(stderr, "Could not receive package!\n");
                    free(buffer);
                    pthread_exit(NULL);
                }
                
                rc += rd;
            }
        }

        rc = 0;
        while (rc < PACKET_SIZE) {
            rd = send(newfd, &buffer[rc], PACKET_SIZE - rc, 0);

            if (rd < 0) {
                fprintf(stderr, "Could not send package!\n");
                free(buffer);
                pthread_exit(NULL);
            }
            
            rc += rd;
        }
    }

    free(buffer);
    close(newfd);
    pthread_exit(NULL);
}

void *work_client(void *argv)
{
    thread_arg_t *arg;
    char *buffer;
    int rc, rd, i;
    struct timeval start, end;

    arg = (thread_arg_t *) argv;

    buffer = (char *) malloc(PACKET_SIZE * sizeof(char));

    init_dataset(buffer, PACKET_SIZE);
    
    gettimeofday(&start, NULL);
    if ((rc = connect(arg->sockfd, arg->srv, arg->addrlen)) < 0) {
        fprintf(stderr, "Could not connect to server!\n");
        pthread_exit(NULL);
    }

    if (arg->mode == MODE_LATENCY) {
        for (i = 0; i < arg->num_messages; ++i) {
            
            rc = 0;
            while (rc < PACKET_SIZE) {
                rd = send(arg->sockfd, &buffer[rc], PACKET_SIZE - rc, 0);

                if (rd < 0) {
                    fprintf(stderr, "Could not send package!\n");
                    free(buffer);
                    pthread_exit(NULL);
                }
                
                rc += rd;
            }

            rc = 0;
            while (rc < PACKET_SIZE) {
                rd = recv(arg->sockfd, &buffer[rc], PACKET_SIZE - rc, 0);
                
                if (rd == 0) {
                    fprintf(stdout, "Connection closed at receive!\n");
                    break;
                }

                if (rd < 0) {
                    fprintf(stderr, "Could not receive package!\n");
                    free(buffer);
                    pthread_exit(NULL);
                }
                
                rc += rd;
            }
        }
    } else {
        for (i = 0; i < arg->num_packets; ++i) {
            
            rc = 0;
            while (rc < PACKET_SIZE) {
                rd = send(arg->sockfd, &buffer[rc], PACKET_SIZE - rc, 0);
                
                if (rd == 0) {
                    fprintf(stdout, "Connection closed at receive!\n");
                    break;
                }

                if (rd < 0) {
                    fprintf(stderr, "Could not receive package!\n");
                    free(buffer);
                    pthread_exit(NULL);
                }
                
                rc += rd;
            }
        }

        rc = 0;
        while (rc < PACKET_SIZE) {
            rd = recv(arg->sockfd, &buffer[rc], PACKET_SIZE - rc, 0);            

            if (rd == 0) {
                fprintf(stdout, "Connection closed at receive!\n");
                break;
            }
            
            if (rd < 0) {
                fprintf(stderr, "Could not receive package!\n");
                free(buffer);
                pthread_exit(NULL);
            }
            
            rc += rd;
        }
    }
    gettimeofday(&end, NULL);

    arg->runtime = ((long) end.tv_sec - (long) start.tv_sec) 
            * 1000000 + (end.tv_usec - start.tv_usec);

    free(buffer);
    pthread_exit(NULL);
}

int main(int argc, char **argv)
{
    int num_threads, mode, type, start_port;
    struct addrinfo hints, *res;
    char ipaddr[INET_ADDRSTRLEN], port[10];
    pthread_t *threads;
    thread_arg_t *args;
    int i, j, rc;
    double throughput;
    long max_runtime, latency;

    // parsing arguments //
    if (argc <= 5 || argc >= 7) {
        fprintf(stderr, "Program usage: ./benchmark-tcp.exe "
                "<num_threads> <mode> <type> <ip_addr> <start_port>\n"
                "where <mode> accepts the following values:\n"
                "\t 0 - Latency experiment\n"
                "\t 1 - Througput experiment\n"
                "where <type> accepts the following values:\n"
                "\t 0 - Client\n"
                "\t 1 - Server\n");
        exit(-1);
    } else {
        num_threads = atoi(argv[1]);
        switch (atoi(argv[2])) {
            case MODE_LATENCY:
                mode = MODE_LATENCY;
                break;
            case MODE_THROUGHPUT:
                mode = MODE_THROUGHPUT;
                break;
            default:
                fprintf(stderr, "Unrecognized mode!\n");
                exit(-1);
        }
        switch (atoi(argv[3])) {
            case TYPE_CLIENT:
                type = TYPE_CLIENT;
                break;
            case TYPE_SERVER:
                type = TYPE_SERVER;
                break;
            default:
                fprintf(stderr, "Unrecognized type!\n");
                exit(-1);
        }
        strcpy(ipaddr, argv[4]);
        start_port = atoi(argv[5]);
    }

    srand(time(NULL));
    args = (thread_arg_t *) malloc(num_threads * sizeof(thread_arg_t));

    // creating and binding (where necessary) the sockets for each thread //
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    
    for (i = 0; i < num_threads; ++i) {
        sprintf(port, "%d", start_port + i);
        
        if ((rc = getaddrinfo(ipaddr, port, &hints, &res)) != 0) {
            fprintf(stderr, "Could not get addrinfo!\n");
            for (j = 0; j <= i; ++j) {
                close(args[j].sockfd);
                free(args[j].srv);
            }
            free(args);
            exit(-2);
        }    

        args[i].sockfd = socket(res->ai_family, res->ai_socktype, 
                res->ai_protocol);

        if (args[i].sockfd < 0) {
            fprintf(stderr, "Could not create socket!\n");
            for (j = 0; j <= i; ++j) {
                close(args[j].sockfd);
                free(args[j].srv);
            }
            free(args);
            exit(-2);
        }

        if (type == TYPE_SERVER) {
            if (bind(args[i].sockfd, res->ai_addr, res->ai_addrlen) < 0) {
                fprintf(stderr, "Could not bind socket!\n");
                for (j = 0; j <= i; ++j) {
                    close(args[j].sockfd);
                free(args[j].srv);
                }
                free(args);
                exit(-2);
            }
        }

        args[i].srv = (struct sockaddr *) malloc(sizeof(struct sockaddr));
        memcpy(args[i].srv, res->ai_addr, sizeof(struct sockaddr));
        args[i].addrlen = res->ai_addrlen;
        args[i].mode = mode;
        args[i].num_messages = NUM_MESSAGES / num_threads;
        args[i].num_packets = NUM_PACKETS / num_threads;
        freeaddrinfo(res);
    }

    // creating and running the threads //
    threads = (pthread_t *) malloc(num_threads * sizeof(pthread_t));

    for (i = 0; i < num_threads; ++i) {
        if (type == TYPE_SERVER) {
            rc = pthread_create(&threads[i], NULL, work_server, 
                    (void *) &args[i]);
        } else {
            rc = pthread_create(&threads[i], NULL, work_client, 
                    (void *) &args[i]);
        }

        if (rc) {
            fprintf(stderr, "Could not create thread!\n");
            for (j = 0; j <= i; ++j) {
                close(args[j].sockfd);
                free(args[j].srv);
            }
            free(args);
            free(threads);
            exit(-3);
        }

    }

    // joining the threads //
    for (i = 0; i < num_threads; ++i) {
        rc = pthread_join(threads[i], NULL);
        if (rc) {
            fprintf(stderr, "Could not join thread!\n");
        }
        close(args[i].sockfd);
        free(args[i].srv);
    }

    if (type == TYPE_CLIENT) {
        max_runtime = args[0].runtime;
        for (i = 1; i < num_threads; ++i) {
            if (max_runtime < args[i].runtime) {
                max_runtime = args[i].runtime;
            }
        }
        
        printf("Elapsed time: %ld ms\n", max_runtime / 1000);
        if (mode == MODE_LATENCY) {
            latency = NUM_MESSAGES;
            latency = (max_runtime / latency) * num_threads;
            printf("Ping-pong message latency: %ld us\n", latency);
        } else {
            throughput = (8.0 * PACKET_SIZE * NUM_PACKETS) 
                    / (double) max_runtime;
            printf("Throughput: %lf Mbps\n", throughput);
        }
    }

    free(threads);
    free(args);

    pthread_exit(NULL);
}
