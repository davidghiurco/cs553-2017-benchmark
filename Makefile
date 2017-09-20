CC = icc
CFLAGS = -march=native -mtune=native -O3 -mavx

.PHONY: cpu
cpu:
	$(CC) $(CFLAGS) -pthread cpu/benchmark.c -o cpu/benchmark.o

run-cpu:
	cpu/benchmark

clean:
	rm -rf *.o
	rm -rf */*.o
