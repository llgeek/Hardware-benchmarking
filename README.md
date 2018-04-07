## CPU, GPU, Memory, Disk and Network Benchmarking

----

to compile:

 ```shell
make
 ```

help information: 

```shell
./AppName -h
```

to run: 

```shell
./AppName	#this is default setting
```

to run all tests: 

```shell
./run_all.sh
```

----

This is the code for benchmarking CPU, GPU, memory, disk and network. To compile the code, first `cd` into the src folder and type `make`. You can also compile soly for each park by input `make cpu` or `make gpu`...


To run the benchmark, you can firstly get the help information by `./AppName -h`, where `AppName` could be `cpu`, `gpu`, `memory`, `disk` or `network`.


Here are some examples:
1. cpu:
  If you want to test "double precision, 4 threads, repeat 10 times", you can input:

```shell
./cpu -o0 -t4 -r10
```

To run cpu with *AVX* instruction, same settings above:
```shell
./cpuavx -o0 -t4 -r10
```

2. gpu:
To test "double precision, bandwidth":
```shell
./gpu -o0 -b
```
To test "integer, speed":
```shell
./gpu -o1
```

3. memory:
To test "write access, block size = 8MB, 8 threads":
```shell
./memory -o1 -b8MB -t8
```

4. disk:
To test "random read, block size = 8KB, 4 threads":
```shell
./disk -o2 -b8KB -t4
```

5. network:
  This is a little complicated here, you need firstly start the server application: (setting could be: TCP, 4 threads)
  `./network -f0 -p0 -t4`
  Then you need to start the client application with same setting (you can also assign the server IP, default is localhost):
  `./network -f1 -p0 -t4 -a127.0.0.1`

Because the bash file cannot support repeatedly close and open the server, so run_all.sh file do not support test network. You have to manually start the server and client.


