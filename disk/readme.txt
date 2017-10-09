Designed by Alexandru Iulian Orhean

1. Benchmark/Application hierarchy
The source code of the application can be found in the 'src' directory. For this
benchmark only one C file (benchmark-lowlevel.c) contains all the code.

2. Build and Compile
In order to build and compile the benchmark just run 'make'. The executables
will be create in the 'bin' directory (by also creating the directory if it does
not exist). The executable's name is benchmark-lowlevel.exe and the following
sections describe how to run the application. The makefile also has a target
that cleans the binary directory.
>>>>
make
make clean

3. Pre-requisite
Before running the benchmark, the input and output files (10GB both) must be
generated first. To do this, you can use the 'create-files.sh' script that is
going to generate a random 10GB input file and copy it as an output file. Since
the names of the input and output files are hard-coded in the benchmark, calling
this script is the best way to generate the files (beware, it might take some
time to create them)(also the size of the files is also hard-coded in the
application):
>>>>
bash create-files.sh

4. Running the benchmark
The benchmark application can be run all together, or each mode and step can be 
run independently. In order to run all the benchmark, you can use the 'run.sh'
script that goes through all the combination of modes, threads and block sizes
and logs the output to three log files: read-write.log, sequential-read.log and
random-read.log:
>>>>
bash run.sh

Beware, the benchmark will take a great deal of time to run, this is why some
log files are already provided. Further on, the script also contains a function
that clears the page cache of the kernel, thus allowing the measurement of pure
hard disk performance.

The applications can be called in the following way, of course by substituting
the variables in the call:
>>>>
./bin/benchmark-lowlevel.exe <num_threads> <block_size> <mode>

<block_size> accepts the following values:
     0 -> 8B block size
     1 -> 8KB block size
     2 -> 8MB block size
     3 -> 80MB block size
<mode> accepts the following values:
     0 -> READ+WRITE operations
     1 -> SEQUENTIAL read
     2 -> RANDOM read

For an example on how to run it, check the run.sh script.

5. Extra
The benchmark also contains the script that generate the plots, which can be
invoked like this:
>>>>
python plot.py <log_file>

Be aware, the python script requires 'matplotlib', and was written for Python
2.7; use at your own discretion.
