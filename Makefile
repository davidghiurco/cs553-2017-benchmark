CC = icc
CFLAGS = -march=native -mtune=native -O3 -mavx

.PHONY: cpu
cpu:
	$(CC) $(CFLAGS) -pthread cpu/benchmark.c -o cpu/benchmark.bin

run-cpu:
	cpu/benchmark

clean:
	rm -rf *.bin
	rm -rf */*.bin
