CC=gcc
CFLAGS=-g -Wall -O2 -lpthread

all: bin
	$(CC) $(CFLAGS) -o bin/benchmark-lowlevel.exe src/benchmark-lowlevel.c

bin:
	mkdir -p bin

clean:
	$(RM) bin/*.exe
