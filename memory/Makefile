CC = c99
CFLAGS = -march=native -mtune=native -O3

memory-host:
	rm -rf *_host.bin
	$(CC) $(CFLAGS) benchmark_host.c -lm -pthread -o benchmark_host.bin

run-memory-host:
	./run_host.sh

memory-gpu: clean
	rm -rf *_gpu.bin
	$(NVCC) benchmark_gpu.c -o benchmark_gpu.bin

run-memory-gpu:
	./run_gpu.sh
