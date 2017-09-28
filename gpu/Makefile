CCX=nvcc
CFLAGS=-ccbin clang-3.8

all: src/benchmark.cu
	$(CCX) $(CFLAGS) --define-macro DOUBLE -o bin/benchmark-double.exe $<
	$(CCX) $(CFLAGS) --define-macro FLOAT -o bin/benchmark-float.exe $<

clean:
	$(RM) bin/*
