CC = c99
CFLAGS = -march=native -mtune=native -O3 -mavx

clean:
	rm -rf *.bin

.PHONY: cpu
cpu: clean
	$(CC) $(CFLAGS) -pthread benchmark.c -o benchmark.bin

run-cpu:
	./run.sh
