CC=g++
NVCC=nvcc
CFLAGS=-std=c++11
LDFLAGS=-pthread
AVXFLAG=-mavx
NVFLAGS=-O0
TARGET=cpu cpuavx memory disk network

all: $(TARGET)

cpu: cpu_benchmark.o 
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

cpuavx: cpu_benchmark_avx.cpp
	$(CC) -o $@ $< $(CFLAGS) $(LDFLAGS) $(AVXFLAG)

#gpu: gpu_benchmark.cu 
#	$(NVCC) -o $@ $< $(NVFLAGS)

memory: memory_benchmark.o 
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

disk: disk_benchmark.o
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

network: network_benchmark.o
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)


%.o: %.cpp
	$(CC) -c -o $@ $< $(CFLAGS) $(LDFLAGS)

clean:
	rm -f *.o $(TARGET)
