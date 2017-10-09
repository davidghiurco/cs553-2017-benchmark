Designed by Alexandru Iulian Orhean

1. Benchmark/Application hierarchy
The source code of the application can be found in the 'src' directory. This
benchmark contains two source files, one for the TCP benchmark (benchmakr-tcp.c)
and the other for the UDP benchmark (benchmark-udp.c).

2. Build and Compile
In order to build and compile the benchmark just run 'make'. The executables
will be create in the 'bin' directory (by also creating the directory if it does
not exist). The executable's names are: benchmark-tcp.exe and benchmark-udp.exe,
and the following sections describe how to run the application. The makefile
also has a target that cleans the binary directory.
>>>>
make
make clean

3. Running the benchmark
The benchmark applications can be run all together, or each mode and step can be 
run independently. In order to run all the benchmark, you can use the 'run.sh'
script that goes through all the combination of modes, threads and types, starts
both the server and the client applications and logs the output to four log
files: tcp-server.log, tcp-client.log, udp-server.log and udp-client.log:
>>>>
bash run.sh

The server log files are used only to store information regarding server side
errors, while the client logs contain the results of the experiments. The script
manages the servers and clients by itself, waiting an appropriate amount of time
in order to start another iteration and allowing enough time for the system to
unbind sockets that were not closed through the application.

Beware, the benchmark will take a great deal of time to run, this is why some
log files are already provided.

The applications can be called in the following way, of course by substituting
the variables in the call:
>>>>
./bin/benchmark-tcp.exe <num_threads> <mode> <type> <ip_addr> <start_port>

where <mode> accepts the following values:
     0 - Latency experiment
     1 - Throughput experiment
where <type> accepts the following values:
     0 - Client
     1 - Server
>>>>
./bin/benchmark-udp.exe <num_threads> <mode> <type> <ip_addr> <start_port>

where <mode> accepts the following values:
     0 - Latency experiment
     1 - Throughput experiment
where <type> accepts the following values:
     0 - Client
     1 - Server

If you want to run the experiment individually, you will have to firstly start
the server, by setting the type to '1', and then start the appropriate client
(the same mode and thread configuration), with the type set to '0'. The start
port represents the first port for the first thread, while the other threads,
will increment the value and use the following one, according to their thread
id. When running both server and client you do not need to give them different
IPs and ports, since the clients are going to be assigned a port by the kernel.
The only reason to pass them, is that they need to know where the server is (IP
and port) for each thread.

For an example on how to run it, check the run.sh script.

4. Extra
The benchmark also contains the script that generate the plots, which can be
invoked like this:
>>>>
python plot.py tcp-client.log udp-client.log

Be aware, the python script requires 'matplotlib', and was written for Python
2.7; use at your own discretion.
