CC = c99
CFLAGS = -march=native -mtune=native -O3 -mavx

.PHONY: memory-host
memory-host:
	rm -rf *_host.bin
	$(CC) $(CFLAGS) -pthread benchmark_host.c -lm -o benchmark_host.bin

run-memory-host:
	./run_host.sh

.PHONY: memory-gpu
memory-gpu: clean
	rm -rf *_gpu.bin
	$(NVCC) benchmark_gpu.c -o benchmark_gpu.bin

run-memory-gpu:
	./run_gpu.sh
