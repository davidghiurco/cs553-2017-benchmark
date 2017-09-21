#!/usr/bin/env bash

cd "$(dirname "$0")"

# This script can be called with an optional command-line parameter $1
# to pass in the desired dimension of the square matrix to benchmark. Default is 1024

# Arrays of input
benchmark_type=(flops iops)
num_threads=(1 2 4 8)

if [ -d "log/" ]; then
	rm -rf log/
	mkdir log
else
	mkdir log
fi

for type in "${benchmark_type[@]}"
do
	touch log/${type}.log
	for threads in "${num_threads[@]}"
	do
		echo "Benchmarking $type with $threads thread(s)"
		RESULT="$(./benchmark.bin ${type} ${threads} $1)"
		echo ${RESULT}
		echo ${RESULT} >> log/${type}.log
		echo ""
	done
done