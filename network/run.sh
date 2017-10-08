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
    for threads in 1 2 4 8
    do
        ./bin/benchmark-udp.exe $threads $mode 1 $ipaddr $port &>> $logsrvudp &
        sleep 1;
        ./bin/benchmark-udp.exe $threads $mode 0 $ipaddr $port &>> $logcltudp &
        wait
        sleep 60
    done
done
