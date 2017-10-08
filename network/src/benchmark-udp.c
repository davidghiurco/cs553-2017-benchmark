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

#define PACKET_SIZE 64 * 1024
#define NUM_MESSAGES 8 * 1024
#define NUM_PACKETS 128 * 1024

#define NUM_TERMINATE_MSGS 32
#define NUM_ATTEMPTS 32

typedef struct thread_arg_t
{
    struct sockaddr_storage *srv;
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
    int rc, rd, i, terminate;

    arg = (thread_arg_t *) argv;

    buffer = (char *) malloc(PACKET_SIZE * sizeof(char));

    terminate = 0;
    if (arg->mode == MODE_LATENCY) {
        for (i = 0; i < arg->num_messages; ++i) {
            memset(buffer, 0, PACKET_SIZE);
            
            rc = 0;
            while (rc < PACKET_SIZE) {
                addrlen = arg->addrlen;
                rd = recvfrom(arg->sockfd, &buffer[rc], PACKET_SIZE - rc + 1, 0,
                        (struct sockaddr *) &clt, &addrlen);
                
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

                if (rc > 0 && buffer[rc - 1] == 0) {
                    terminate = 1;
                    break;
                }
            }

            if (terminate) {
                break;
            }

            rc = 0;
            while (rc < PACKET_SIZE) {
                addrlen = arg->addrlen;
                rd = sendto(arg->sockfd, &buffer[rc], PACKET_SIZE - rc, 0,
                        (struct sockaddr *) &clt, addrlen);

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
                addrlen = arg->addrlen;
                rd = recvfrom(arg->sockfd, &buffer[rc], PACKET_SIZE - rc, 0,
                        (struct sockaddr *) &clt, &addrlen);
                
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
                
                if (rc > 0 && buffer[rc - 1] == 0) {
                    terminate = 1;
                    break;
                }
            }
            
            if (terminate) {
                break;
            }
        }
    }

    free(buffer);
    pthread_exit(NULL);
}

void *work_client(void *argv)
{
    thread_arg_t *arg;
    struct sockaddr_storage clt;
    socklen_t addrlen;
    char *dataset, *buffer;
    int rc, rd, i, num_attempts;
    struct timeval start, end;

    arg = (thread_arg_t *) argv;

    dataset = (char *) malloc(PACKET_SIZE * arg->num_packets * sizeof(char));

    init_dataset(dataset, PACKET_SIZE * arg->num_packets);
    
    gettimeofday(&start, NULL);

    if (arg->mode == MODE_LATENCY) {
        for (i = 0; i < arg->num_messages; ++i) {
            buffer = &dataset[i * PACKET_SIZE];
            
            rc = 0;
            while (rc < PACKET_SIZE) {
                rd = sendto(arg->sockfd, &buffer[rc], PACKET_SIZE - rc, 0, 
                        (struct sockaddr *) arg->srv, arg->addrlen);

                if (rd < 0) {
                    fprintf(stderr, "Could not send package!\n");
                    free(dataset);
                    pthread_exit(NULL);
                }
                
                rc += rd;
            }

            num_attempts = 0;
            rc = 0;
            while (rc < PACKET_SIZE && num_attempts < NUM_ATTEMPTS) {
                addrlen = arg->addrlen;
                rd = recvfrom(arg->sockfd, &buffer[rc], PACKET_SIZE - rc, 0,
                        (struct sockaddr *) &clt, &addrlen);
                
                if (rd == 0) {
                    fprintf(stdout, "Connection closed at receive!\n");
                    break;
                }

                if (rd < 0) {
                    fprintf(stderr, "Could not receive package!\n");
                    free(dataset);
                    pthread_exit(NULL);
                }
                
                rc += rd;
                num_attempts += 1;
            }
        }
        
        buffer = &dataset[0];
        memset(buffer, 0, PACKET_SIZE);
        for (i = 0; i < NUM_TERMINATE_MSGS; ++i) {
            
            rc = 0;
            while (rc < PACKET_SIZE) {
                rd = sendto(arg->sockfd, &buffer[rc], PACKET_SIZE - rc, 0,
                        (struct sockaddr *) arg->srv, arg->addrlen);

                if (rd < 0) {
                    fprintf(stderr, "Could not send package!\n");
                    free(dataset);
                    pthread_exit(NULL);
                }
                
                rc += rd;
            }
        }
    } else {
        for (i = 0; i < arg->num_packets; ++i) {
            buffer = &dataset[i * PACKET_SIZE];
            
            rc = 0;
            while (rc < PACKET_SIZE) {
                rd = sendto(arg->sockfd, &buffer[rc], PACKET_SIZE - rc, 0,
                        (struct sockaddr *) arg->srv, arg->addrlen);

                
                if (rd == 0) {
                    fprintf(stdout, "Connection closed at send!\n");
                    break;
                }

                if (rd < 0) {
                    fprintf(stderr, "Could not send package!\n");
                    free(dataset);
                    pthread_exit(NULL);
                }
                
                rc += rd;
            }
        }

        buffer = &dataset[0];
        memset(buffer, 0, PACKET_SIZE);
        for (i = 0; i < NUM_TERMINATE_MSGS; ++i) {
            
            rc = 0;
            while (rc < PACKET_SIZE) {
                rd = sendto(arg->sockfd, &buffer[rc], PACKET_SIZE - rc, 0,
                        (struct sockaddr *) arg->srv, arg->addrlen);

                if (rd < 0) {
                    fprintf(stderr, "Could not send package!\n");
                    free(dataset);
                    pthread_exit(NULL);
                }
                
                rc += rd;
            }
        }
    }
    gettimeofday(&end, NULL);

    arg->runtime = ((long) end.tv_sec - (long) start.tv_sec) 
            * 1000000 + (end.tv_usec - start.tv_usec);

    free(dataset);
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

    // parsing arguments //
    if (argc <= 5 || argc >= 7) {
        fprintf(stderr, "Program usage: ./benchmark-tcp.exe "
                "<num_threads> <mode> <type> <ip_addr> <start_port>\n"
                "where <mode> accepts the following values:\n"
                "\t 0 - Latency experiment\n"
                "\t 1 - Througput experiment\n"
                "where <type accepts the following values:\n"
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
    hints.ai_socktype = SOCK_DGRAM;
    
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

        args[i].srv = (struct sockaddr_storage *) 
                malloc(sizeof(struct sockaddr_storage));
        memcpy(args[i].srv, res->ai_addr, sizeof(struct sockaddr_storage));
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
        for (i = 0; i < num_threads; ++i) {
            printf("Thread %d runtime: %ld us\n", i, args[i].runtime);
        }
    }

    free(threads);
    free(args);

    pthread_exit(NULL);
}
