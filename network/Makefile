CC=gcc
CFLAGS=-g -Wall -O2 -lpthread

all: bin
	$(CC) $(CFLAGS) -o bin/benchmark-tcp.exe src/benchmark-tcp.c
	$(CC) $(CFLAGS) -o bin/benchmark-udp.exe src/benchmark-udp.c

bin:
	mkdir -p bin

clean:
	$(RM) bin/*.exe
