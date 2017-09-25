#!/usr/bin/env bash

cd "$(dirname "$0")"

# Arrays of input
benchmark_type=(read_and_write seq_read_access random_read_access)

# 1 B, 1 KB, 1 MB, 10 MB
block_size=(1 1000 1000000 10000000)

num_threads=(1 2 4 8)

if [ -d "log/" ]; then
	rm -rf log/
	mkdir log/memcpy
	mkdir log/seq_read_access
	mkdir log/random_read_access
else
	mkdir log/memcpy
	mkdir log/seq_read_access
	mkdir log/random_read_access
fi

for type in "${benchmark_type[@]}"
do
	for blk_size in "${block_size[@]}"
	do
		touch log/${type}/blk-${blk_size}b.log
		for threads in "${num_threads[@]}"
		do
			echo "Benchmarking $type with $blk_size B blocks and $threads thread(s)"
			RESULT="$(./benchmark_host.bin ${type} ${blk_size} ${threads})"
			echo ${RESULT}
			echo ${RESULT} >> log/${type}.log
			echo ""
		done
	done
done