# cs553-2017-benchmark
Cloud computing (Fall 2017 @ Illinois Institute of Technology) benchmarking suite for Chameleon

## CPU Instructions

### Compiling

```bash
cd cpu/
make cpu
```


### Running all experiments

```bash
make run-cpu
```

This will run all 8 experiments that have to be written for this part of the assignment. 
(The extra credit experiments are not done).
* Note: the experiments in this section are performed using Intel AVX instructions.  
The binary is optimized to use these instruction sets

### Compiling and running HPL

First, install and setup Spack

```bash
cd ~/
git clone https://github.com/llnl/spack.git
export PATH="~/spack/bin:$PATH"
sudo yum install environment-modules -y
source /etc/profile.d/modules.sh
source ~/spack/share/spack/setup-env.sh
```
	

Install HPL using spack

```bash
spack install hpl cflags="-march=native -mtune=native -O3" ^intel-mkl ^intel-mpi
```
	
Load the HPL binary into the PATH

```bash
spack load hpl
```

Now we need to cd into the directory where the hpl binary (named "xhpl") and HPL.dat input file reside

It is time to write up the input file for HPL. The most important thing is to determine the problem size (N):
```bash
cd $(dirname $(which xhpl))
```


On x.large KVM instances, the total RAM is 16,384 MB. 

To calculate the problem size for HPL the following equation is used:

Memory size for HPL 

=  sqrt({phyiscal memory in bytes} * 0.80 / 8 )

= sqrt(16,384 * 1,000,000 * 0.80 / 8)
                                
= 40,477
* Note: This will take up roughly 80% of available RAM
* Note2: The division by 8 comes from the fact that a double-precision floating point number occupies 8 bytes

Images and further information on HPL.dat (the input file), as well as the output and results are
provided in the report.

Now to run hpl, we need to have an MPI distribution available in the path.
We already installed it using spack. Load it now

```bash
spack load intel-mpi
```

To run HPL (from within the /bin directory containing the xhpl binary):

```bash
cd $(dirname $(which xhpl))
mpirun -n 8 ./xhpl
```


## Memory Instructions

### Compiling

```bash
cd memory/
make memory-host
```

### Running all experiments

```bash
make run-memory-host
```

### Compiling and running STREAM

Download from GitHub
```bash
git clone https://github.com/jeffhammond/STREAM.git


```

Compile
* We will be using 30 million elements in the problem size (3x larger than the default)

```bash
cd STREAM/
gcc -march=native -mtune=native -O3 -fopenmp -D_OPENMP -D STREAM_ARRAY_SIZE=30000000 stream.c -o stream

```

Before running, we need to tell Stream how many threads to use. We will use 8 because x.xlarge KVM instances on Chameleon have 8 VCPUs. 

```bash
export OMP_NUM_THREADS=8
```

To run (from the top level of the git repository downloaded earlier):

```bash
./stream
```
