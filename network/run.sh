#!/bin/bash

logsrvtcp="tcp-server.log"
logsrvudp="udp-server.log"
logclttcp="tcp-client.log"
logcltudp="udp-client.log"

ipaddr=127.0.0.1
port=11155

echo -n "" > $logsrvtcp
echo -n "" > $logsrvudp
echo -n "" > $logclttcp
echo -n "" > $logcltudp

for mode in {0..1}
do
    if [ $mode -eq 0 ]
    then
        echo "Running latency experiment" >> $logsrvtcp
        echo "Running latency experiment" >> $logclttcp
    else
        echo "Running throughput experiment" >> $logsrvtcp
        echo "Running throughput experiment" >> $logclttcp
    fi
    
    for threads in 1 2 4 8
    do
        ./bin/benchmark-tcp.exe $threads $mode 1 $ipaddr $port &>> $logsrvtcp &
        sleep 1;
        ./bin/benchmark-tcp.exe $threads $mode 0 $ipaddr $port &>> $logclttcp &
        wait
        sleep 60
    done
done

for mode in {0..1}
do
    if [ $mode -eq 0 ]
    then
        echo "Running latency experiment" >> $logsrvudp
        echo "Running latency experiment" >> $logcltudp
    else
        echo "Running throughput experiment" >> $logsrvudp
        echo "Running throughput experiment" >> $logcltudp
    fi
    
    for threads in 1 2 4 8
    do
        ./bin/benchmark-udp.exe $threads $mode 1 $ipaddr $port &>> $logsrvudp &
        sleep 1;
        ./bin/benchmark-udp.exe $threads $mode 0 $ipaddr $port &>> $logcltudp &
        wait
        sleep 60
    done
done
