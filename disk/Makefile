CC=gcc
CFLAGS=-g -Wall -lpthread

all: bin
	$(CC) $(CFLAGS) -o bin/benchmark-lowlevel.exe src/benchmark-lowlevel.c

bin:
	mkdir -p bin

clean:
	$(RM) bin/*.exe
