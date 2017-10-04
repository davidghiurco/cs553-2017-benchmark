#!/usr/bin/env bash

cd "$(dirname "$0")"

# Arrays of input
benchmark_type=(read_and_write seq_write_access random_write_access)

# 8 B, 8 KB, 8 MB, 80 MB
block_size=(8 8000 8000000 80000000)

num_threads=(1 2 4 8)

if [ -d "log/" ]; then
	rm -rf log/
	mkdir log/
	mkdir log/read_and_write
	mkdir log/seq_write_access
	mkdir log/random_write_access
else
	mkdir log/
	mkdir log/read_and_write
	mkdir log/seq_write_access
	mkdir log/random_write_access
fi

for type in "${benchmark_type[@]}"
do
	for blk_size in "${block_size[@]}"
	do
		# touch log/${type}/blk-${blk_size}b.log
		for threads in "${num_threads[@]}"
		do
			echo "Benchmarking $type with $blk_size B blocks and $threads thread(s)"
			RESULT="$(./benchmark_host.bin ${type} ${blk_size} ${threads})"
			echo ${RESULT}
			echo ${RESULT} >> log/${type}/blk-${blk_size}b.log
			echo ""
		done
	done
done