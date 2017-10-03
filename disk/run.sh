#!/bin/bash

function clear_cache {
	sync
	sudo sh -c 'echo 3 > /proc/sys/vm/drop_caches'
}

for mode in {0..2}
do
    logfile="temp.log"
    if [ mode -eq 0 ]
    then
        logfile="read-write.log"
    elif [ mode -eq 1 ]
    then
        logfile="sequential-read.log"
    elif [ mode -eq 2 ]
        logfile="random-read.log"
    fi

    echo "" > $logfile

    for size in {0..3}
    do
        if [ size -eq 0 ]
        then
            echo "Running 8B block size experiment" >> $logfile
        elif [ size -eq 1 ]
        then
            echo "Running 8KB block size experiment" >> $logfile
        elif [ size -eq 2 ]
        then
            echo "Running 8MB block size experiment" >> $logfile
        elif [ size -eq 3 ]
        then
            echo "Running 80MB block size experiment" >> $logfile
        fi

        for threads in 1 2 4 8
        do
	        clear_cache
            ./bin/benchmark-lowlevel.exe $threads $size $mode >> $logfile
        done
    done
done
