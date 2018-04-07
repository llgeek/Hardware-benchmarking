/**
 * @file memory_benchmark.cpp
 * benchmark for memory reading and writing
 *
 * author: Linlin Chen
 * lchen96@hawk.iit.edu
 */

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sys/time.h>	
#include <unistd.h>		//getopt
#include <cstring>		//memset and memcpy
#include <string>
#include <algorithm>	//std::generate
#include <random>		//random function
#include <cmath>
#include "memory_benchmark.h"

using namespace std;



/**
 * user manual, help user how to give input
 */
void helper (char *arg) {
	cout<<arg<<": Memory benchmark tool, Version 0.0.1 (lchen96@hawk.iit.edu)"<<endl<<endl;
	cout<<"usage:\t"<<arg<<" [-h] [-o <operation>] [-t <threads>] [-s <datasize>] [-b <blocksize] [-r <repeats>]"<<endl;
	cout<<"Arguments:"<<endl;
	cout<<"\t-h\tlist available commands"<<endl;
	cout<<"\t-o\toperation type, read&write=0 (defualted), sqtwrite=1,rdmwrite=2"<<endl;
	cout<<"\t-t\tnumber of threads ( <= "<<MAXTHREADS<<") [default = 1]"<<endl;
	cout<<"\t-s\tmemory size to be operated, ending with B/KB/MB/GB, default with B [default = 1.25GB]"<<endl;
	cout<<"\t-b\tblock size, ending with B/KB/MB [default = 8B]"<<endl;
	cout<<"\t-r\tnumber of repeated benchmark tests[default = 1]"<<endl;
	cout<<endl;

}


/**
 * handle with user input
 * allow the input with size unit, like B, KB, MB, GB
 * transform the input into size in Byte
 * @param  input: user's input, ending with size unit
 * @return       size in Byte if sucessful, otherwise return -1
 */
long getSizeInByte (string input) {
	string::size_type sz;	//alias of size_t
	long value;

	if ((value = stol(input, &sz)) != 0) {
		if (input.compare(input.size() - 2, 2, "GB") == 0) 	//GB
			return GB_IN_BYTE(value);
		else if (input.compare(input.size() - 2, 2, "MB") == 0)	//MB
			return MB_IN_BYTE(value);
		else if (input.compare(input.size() - 2, 2, "KB") == 0)		//KB
			return KB_IN_BYTE(value);
		else if (input.compare(input.size() - 1, 1, "B") == 0)	//B
			return value;
		else		//input without size unit, default with Byte
			return value;
	} else {
		cout<<"Invalid size valie"<<endl;
		return -1;
	}
}


int main (int argc, char *argv[]) {
	/*
	use getopt function to parse the input arguments
	 */
	int c;
	int flag;
	// only accepts "hfitlr" arguments, where for "tlr" the argument can be optional
	while ((c = getopt (argc, argv, ":ho::t::s::b::r::")) != -1) 
		switch (c) {
			case 'h':
				helper(argv[0]);
				exit(0);
				break;
			case 'o':
				flag = stoi(optarg);
				if (flag == 0)
					op_type = RDW;
				else if (flag == 1)
					op_type = SW;
				else if (flag == 2)
					op_type = RMW;
				else {
					cerr<<"option type can only be 0,1 or 2!\n"<<endl;
					helper(argv[0]);
					exit(1);
				}
				break;
			case 't':
				if ((thread_num = atoi(optarg)) > MAXTHREADS) {
					cout<<"Exceed the supported thread limit!"<<endl;
					exit(2);
				}
				break;
			case 's':
				if ((data_size = getSizeInByte (optarg)) == -1) {
					cout<<"Invalid data size input\n"<<endl;
					helper(argv[0]);
					exit(1);
				}
				break;
			case 'b':
				if ((block_size = getSizeInByte(optarg)) == -1) {
					cout<<"Invalid block size input\n"<<endl;
					helper(argv[0]);
					exit(1);
				}
				if (block_size > MAXBLOCKSIZE) {
					cout<<"Exceed the supported block size! ( <= "<<BYTE_IN_MB(MAXBLOCKSIZE)<<"MB)"<<endl;
					exit(1);
				}
				break;
			case 'r':
				repeat_num = stoi(optarg);
				break;
			case ':':	/*missing option argument, using default value*/
				if (optopt == 'o')
					op_type = RDW;
				else if (optopt == 't')
					thread_num = 1;
				else if (optopt == 's')
					data_size = DEFAULTDATASIZE;
				else if (optopt == 'b')
					block_size = EBBLOCK;
				else if (optopt == 'i')
					repeat_num = 1;
				break;
			case '?':	/* invalid option*/
			default:
				cerr<<"Invalid arguments!\n"<<endl;
				helper(argv[0]);
				abort();
		}



		/*
		print user's input information
		 */	
		cout<<"\nThe benchmarking begins with:"
			<<"\n\tOperation:\t\t"<<op[op_type]
			<<"\n\t#Thread:\t\t"<<thread_num
			<<"\n\tData size:\t\t"<<BYTE_IN_GB(data_size)<<" GB"
			<<"\n\tBlock size:\t\t";
		if (block_size < ONEKB)
				cout<<block_size<<"B\t";
		else if (block_size < ONEMB)
			cout<<BYTE_IN_KB(block_size)<<"KB";
		else if (block_size < ONEGB)
			cout<<BYTE_IN_MB(block_size)<<"MB";
		else
			cout<<BYTE_IN_GB(block_size)<<"GB";
		cout<<"\n\t#Iteration:\t\t"<<repeat_num<<endl<<endl<<endl;

		//to reducce the memory consumption, allocating space based on benchmark purpose
		//to accelerate speed, generating random values in advance
		memRangePerThrd = new size_t[thread_num];
		memStartPerThrd = new size_t[thread_num];
		numOptPerThrd = new size_t[thread_num];

		size_t perrange = (size_t) data_size / thread_num;
		for (int i = 0; i < thread_num-1; i++) {
			memRangePerThrd[i] = perrange;
			memStartPerThrd[i] = i * perrange;
			numOptPerThrd[i] = perrange / block_size;
		}
		memRangePerThrd[thread_num-1] = data_size - perrange * (thread_num - 1);
		memStartPerThrd[thread_num-1] = perrange * (thread_num - 1);
		numOptPerThrd[thread_num-1] = memRangePerThrd[thread_num-1] / block_size;


		if (op_type == RDW) {		//allocate memory for reading
			sourceMem = new char[data_size];
			memset(sourceMem, '1', data_size);
		}
		// } else {				//allocate memory for storing data, which will be directly written
		// 	dataSample = new char[block_size];
		// 	memset(dataSample, '2', block_size);
		// }

		if (op_type == RMW) {		//generate random numbers in advance
			rdmIndex = new size_t* [thread_num];
			int sindex = 0;
			for (int i = 0; i < thread_num; i ++) {
				// random_device rd;		//obtain a seed for the random number engine
				// mt19937 gen(rd());		//mersenne_twister_engine seeded
				// uniform_int_distribution<size_t> dis(0, memRangePerThrd[i] - 1 - block_size);	//uniform distribution
				
				rdmIndex[i] = new size_t[numOptPerThrd[i]];

				// for (int j = 0; j < numOptPerThrd[i]; j++) {
				// 	rdmIndex[i][j] = dis(gen);
				// }
				//generate(rdmIndex + sindex, rdmIndex + sindex + numOptPerThrd[i], dis);	//generate random numbers in advance
				//sindex += numOptPerThrd[i];
			}
		}
		
		destMem = new char[data_size];


		//float *runtime = (float *) malloc (sizeof(float) * repeat_num); 
		float *runtime = new float[repeat_num];

		cout<<"Mem\tOpType\t#Thread\tDataSize\tBlockSize\tThroughput(MB/sec)\tLatency(us)"<<endl;
		for (int i = 0; i < repeat_num; i++) {

			if (op_type == RMW) {
				for (int j = 0; j < thread_num; j++) {
					random_device rd;		//obtain a seed for the random number engine
					mt19937 gen(rd());		//mersenne_twister_engine seeded
					uniform_int_distribution<size_t> dis(0, memRangePerThrd[i] - 1 - block_size);	//uniform distribution
					for (int k = 0; k < numOptPerThrd[j]; k++) {
						rdmIndex[i][j] = dis(gen);
					}
				}
			}

			runtime[i] = memory_benchmark();
			cout<<"#Iter "<<i<<"\t"<<op[op_type]<<"\t"<<thread_num
				<<"\t"<<BYTE_IN_GB(data_size)<<"GB\t";

			if (block_size < ONEKB)
				cout<<block_size<<"B\t";
			else if (block_size < ONEMB)
				cout<<BYTE_IN_KB(block_size)<<"KB\t";
			else if (block_size < ONEGB)
				cout<<BYTE_IN_MB(block_size)<<"MB\t";
			else
				cout<<BYTE_IN_GB(block_size)<<"GB\t";

			cout<<BYTE_IN_MB(data_size)/runtime[i]<<"MB/s\t"
				<<runtime[i]*1e6<<"us"<<endl;

				
		}


		
		/*
		free space before exiting application
		 */
		delete[] runtime;
		if (op_type == RMW) {
			for (int i = 0; i < thread_num; i++)
				delete[] rdmIndex[i];
			delete[] rdmIndex;
		}
//		delete[] dataSample;
		if (op_type == RDW)
			delete[] sourceMem;

		delete[] destMem;

		delete[] memRangePerThrd;
		delete[] memStartPerThrd;
		delete[] numOptPerThrd;

		return 0;

}


/**
 * memory benchmark function implenemtation
 * @return benchmarking time in seconds
 */
double memory_benchmark () {
	struct timeval starttime, endtime;
	long runtime;
	pthread_t *threads = new pthread_t [thread_num];
	int* thrdID = new int[thread_num];
	for (int i = 0; i < thread_num; i++)
		thrdID[i] = i; 

	gettimeofday(&starttime, NULL);
	if (op_type == RDW) {
		for (int tid = 0; tid < thread_num; tid++)
			//pthread_create(&threads[tid], NULL, readWrite, (void *)&tid);
			pthread_create(&threads[tid], NULL, readWrite, (void *)(thrdID + tid));
	} else if (op_type == SW) {
		for (int tid = 0; tid < thread_num; tid++)
			pthread_create(&threads[tid], NULL, sqtialWrite, (void *)(thrdID + tid));
	} else if (op_type == RMW) {
		for (int tid = 0; tid < thread_num; tid++)
			pthread_create(&threads[tid], NULL, rdmWrite, (void *)(thrdID + tid));
	} else {
		cerr<<"Invalid structions! opType can only be 0,1,2!"<<endl;
		abort();
	}
	for (int i = 0; i < thread_num; i++) {
		pthread_join(threads[i], NULL);
	}
	gettimeofday(&endtime, NULL);

	delete[] threads;
	delete[] thrdID;
	
	runtime = (endtime.tv_sec - starttime.tv_sec) * 1000000L + (endtime.tv_usec - starttime.tv_usec);	//time in microsecond
	return runtime/1000000.0;	//time in seconds 

}


/**
 * sequential read and write thread implementation
 * @param  argv thread ID
 * @return      exit current thread once it completes without being suspended
 */
void *readWrite (void *argv) {
//	int crtThrdID = pthread_self();		//get the calling thread ID
	int crtThrdID = *(int *)argv;
//	int crtThrdID = 0;
	for (size_t i = 0; i < numOptPerThrd[crtThrdID]; i++) {
		memcpy(destMem + memStartPerThrd[crtThrdID] + i*block_size, sourceMem + memStartPerThrd[crtThrdID] + i*block_size, block_size);
	}
	//pthread_exit(NULL);
	return NULL;
}

/**
 * sequential write thread implementation
 * @param  argv thread ID
 * @return      exit current thread once it completes without being suspended
 */
void *sqtialWrite (void *argv) {
//	int crtThrdID = pthread_self();
	int crtThrdID = *(int *) argv;
	for (size_t i = 0; i < numOptPerThrd[crtThrdID]; i++) {
		memset(destMem + memStartPerThrd[crtThrdID] + i*block_size, '1', block_size);
	}
	//pthread_exit(NULL);
	return NULL;
}


/**
 * random write thread implementation
 * @param  argv thread ID
 * @return      exit current thread once it completes without being suspended
 */
void *rdmWrite (void *argv) {
//	int crtThrdID = pthread_self();
	int crtThrdID = *(int *) argv;
	for (size_t i = 0; i < numOptPerThrd[crtThrdID]; i++) {
		memset(destMem + memStartPerThrd[crtThrdID] + rdmIndex[crtThrdID][i], '1', block_size);
	}
	//pthread_exit(NULL);
	return NULL;
}






