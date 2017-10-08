## Compile host memory benchmark

```bash
make memory-host
```


## Run all host memory benchmarks

```bash
make run-memory-host
```
	
## Running an individual experiment on the benchmark binary

To run an individual experiment, the usage is:
```bash
./benchmark_host.bin <operation> <block size> <num threads>
```
where __operation__ is either:
* read_and_write
* seq_write_access
* random_write_access