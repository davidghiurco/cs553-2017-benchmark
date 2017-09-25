CC = icc
CFLAGS = -march=native -mtune=native -O3 -mavx

clean:
	rm -rf *.bin
	rm -rf */*.bin

.PHONY: cpu
cpu: clean
	$(CC) $(CFLAGS) -pthread cpu/benchmark.c -o cpu/benchmark.bin

run-cpu:
	cpu/run.sh

.PHONY: memory
memory: clean
	$(CC) $(CFLAGS) -pthread memory/benchmark_host.c -o memory/benchmark_host.bin