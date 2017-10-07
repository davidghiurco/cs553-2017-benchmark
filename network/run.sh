#!/bin/bash

logsrvtcp="tcp-server.log"
logsrvudp="udp-server.log"
logclttcp="tcp-client.log"
logcltudp="udp-client.log"

ipaddr=127.0.0.1
port=11155

echo "" > $logsrvtcp
echo "" > $logsrvudp
echo "" > $logclttcp
echo "" > $logcltudp

for mode in {0..1}
do
    for threads in 1 2 4 8
    do
        ./bin/benchmark-tcp.exe $threads $mode 1 $ipaddr $port 2>&1 $logsrvtcp &
        sleep 1
        ./bin/benchmark-tcp.exe $threads $mode 0 $ipaddr $port 2>&1 $logclttcp &
        sleep 10
    done
done

for mode in {0..1}
do
    for threads in 1 2 4 8
    do
        ./bin/benchmark-udp.exe $threads $mode 1 $ipaddr $port 2>&1 $logsrvudp &
        sleep 1
        ./bin/benchmark-udp.exe $threads $mode 0 $ipaddr $port 2>&1 $logcltudp &
        sleep 10
    done
done
