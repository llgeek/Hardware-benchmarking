/**
 * @file disk_benchmark.cpp
 * benchmark for disk reading and writing
 *
 * author: Linlin Chen
 * lchen96@hawk.iit.edu
 */
#define _LARGEFILE64_SOURCE		//support large file operations (> 4GB)
#define _FILE_OFFSET_BITS 64
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sys/time.h>
#include <unistd.h>		
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>
#include <algorithm>	//std::generate
#include <random>		//random function
#include "disk_benchmark.h"

using namespace std;

/**
 * user manual, help user how to give input
 */
void helper (char *arg) {
	cout<<arg<<": Disk benchmark tool, Version 0.0.1 (lchen96@hawk.iit.edu)"<<endl<<endl;
	cout<<"usage:\t"<<arg<<" [-h] [-i] [-o <operation>] [-t <threads>] [-s <datasize>] [-b <blocksize] [-r <repeats>]"<<endl;
	cout<<"Arguments:"<<endl;
	cout<<"\t-h\tlist available commands"<<endl;
	cout<<"\t-i\tignore file craetion (test file already exists)"<<endl;
	cout<<"\t-o\toperation type, read&write=0 (defualted), sqtread=1, rdmread=2"<<endl;
	cout<<"\t-t\tnumber of threads ( <= "<<MAXTHREADS<<") [default = 1]"<<endl;
	cout<<"\t-s\tfile size to be operated, ending with B/KB/MB/GB (>= "<<BYTE_IN_GB(MINDATASIZE)<<"GB) [default = 10GB]"<<endl;
	cout<<"\t-b\tblock size, ending with B/KB/MB, default with B (<= "<<BYTE_IN_MB(MAXBLOCKSIZE)<<"MB) [default = 8B]"<<endl;
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
	// only accepts "hiotsbi" arguments, where for "otsbr" the argument can be optional
	while ((c = getopt (argc, argv, ":hio::t::s::b::r::")) != -1) 
		switch (c) {
			case 'h':
				helper(argv[0]);
				exit(0);
			case 'i':
				skip_createfile = true;
				break;
			case 'o':
				flag = stoi(optarg);
				if (flag == 0)
					op_type = RDW;
				else if (flag == 1)
					op_type = SR;
				else if (flag == 2)
					op_type = RR;
				else {
					cerr<<"option type can only be 0,1 or 2!\n"<<endl;
					helper(argv[0]);
					exit(1);
				}
				break;
			case 't':
				if ((thread_num = atoi(optarg)) > MAXTHREADS) {
					cout<<"Exceed the supported thread limit! ( <= "<<MAXTHREADS<<")"<<endl;
					exit(2);
				}
				break;
			case 's':
				if ((data_size = getSizeInByte (optarg)) == -1) {
					cout<<"Invalid data size input\n"<<endl;
					helper(argv[0]);
					exit(1);
				}
				if (data_size < GB_IN_BYTE(1L)) {
					cout<<"File size is too small ( >= "<<BYTE_IN_GB(MINDATASIZE)<<"GB)"<<endl;
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
				else if (optopt == 'r')
					repeat_num = 1;
				break;
			case '?':	/* invalid option*/
			default:
				cerr<<"Invalid arguments!\n"<<endl;
				helper(argv[0]);
				abort();
		}

		//if the block size is smaller than 100Byte
		if (block_size < 100)	
			data_size = GB_IN_BYTE(1L);		//set data size to 1GB
		else if (block_size < KB_IN_BYTE(100L))	//if block size smaller than 100KB
			data_size = GB_IN_BYTE(4L);		//set data size to 4GB


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
		fileRangePerThrd = new size_t[thread_num];
		fileStartPerThrd = new size_t[thread_num];
		numOptPerThrd = new size_t[thread_num];

		size_t perrange = (size_t) data_size / thread_num;
		for (int i = 0; i < thread_num-1; i++) {
			fileRangePerThrd[i] = perrange;
			fileStartPerThrd[i] = i * perrange;
			numOptPerThrd[i] = perrange / block_size;
		}
		fileRangePerThrd[thread_num-1] = data_size - perrange * (thread_num - 1);
		fileStartPerThrd[thread_num-1] = perrange * (thread_num - 1);
		numOptPerThrd[thread_num-1] = fileRangePerThrd[thread_num-1] / block_size;


		//create the write file if it's to benchmark read+write
		//rely on POSIX file operations to accelerate speed
		if (op_type == RDW) {
			if ((writeFile = open(WTFILENAME, O_WRONLY | O_CREAT  | O_NONBLOCK, (mode_t)0666)) == -1) {
				cerr<<"Cannot create file: "<<WTFILENAME<<endl;
				exit(3);
			}
			if (skip_createfile == false) {//not skip the file creation, initialize the file content/size
				if (lseek(writeFile, data_size-1, SEEK_SET) == -1 || write(writeFile, "", 1) < 0) {
					close(writeFile);
					cerr<<"Cannot stretch file: "<<WTFILENAME<<" to "<<BYTE_IN_GB(data_size)<<"GB"<<endl;
					exit(3);
				}
			}
		} 
		//allocate space for read buffer to store raed content for each thread
		bufferStore = new char*[thread_num];
		for (int i = 0; i < thread_num; i++) 
			bufferStore[i] = new char[block_size]; 
		


		//allocate the disk space for the file to be read, and initialize its content
		//so firstly open the file for writing only
		//if skip_createfile, then directly open file without initializtion
		if (skip_createfile == false) {
			if ((readFile = open(RDFILENAME, O_WRONLY | O_CREAT | O_NONBLOCK, (mode_t)0666)) == -1) {
				cerr<<"Cannot create file: "<<RDFILENAME<<endl;
				exit(3);
			}
			// FILE* tmpreadFile;
			// if ((tmpreadFile = fopen(RDFILENAME, "wb")) == NULL) {
			// 	cerr<<"Cannot create file: "<<RDFILENAME<<endl;
			// 	exit(3);
			// }
			//fill the read file with content '1'
			char* tmptowrite = new char[MB_IN_BYTE(100L)];
			memset(tmptowrite, '1', MB_IN_BYTE(100L));
			for (int i = 0; i < data_size / MB_IN_BYTE(100L); i++) {
				//cout<<(fwrite(tmptowrite, sizeof(char), MB_IN_BYTE(1L), tmpreadFile));
				write(readFile, tmptowrite, MB_IN_BYTE(100L));
			}
			if (data_size % MB_IN_BYTE(100L) != 0) {
				//fwrite(tmptowrite, sizeof(char), data_size % MB_IN_BYTE(1L), tmpreadFile);
				write(readFile, tmptowrite, data_size % MB_IN_BYTE(100L));
			}
			//fclose(tmpreadFile);
			close(readFile);
			delete[] tmptowrite;
		}
		
		//open the file for read only
		if ((readFile = open(RDFILENAME, O_RDONLY | O_CREAT  | O_NONBLOCK, (mode_t)0666)) == -1) {
			cerr<<"Cannot open file: "<<RDFILENAME<<endl;
			exit(3);
		}

		
		if (op_type == RR) {		//generate random numbers in advance
			rdmIndex = new size_t* [thread_num];
			for (int i = 0; i < thread_num; i ++) {
				rdmIndex[i] = new size_t[numOptPerThrd[i]];
			}
		}



		//float *runtime = (float *) malloc (sizeof(float) * repeat_num); 
		float *runtime = new float[repeat_num];

		cout<<"Disk\tOpType\t#Thread\tFileSize\tBlockSize\tThroughput(MB/sec)\tLatency(us)"<<endl;
		for (int i = 0; i < repeat_num; i++) {
			if (op_type == RR) {
				for (int j = 0; j < thread_num; j++) {
					random_device rd;		//obtain a seed for the random number engine
					mt19937 gen(rd());		//mersenne_twister_engine seeded
					uniform_int_distribution<size_t> dis(0, fileRangePerThrd[i] - 1 - block_size);	//uniform distribution
					for (int k = 0; k < numOptPerThrd[j]; k++) {
						rdmIndex[i][j] = dis(gen);
					}
				}
			}

			runtime[i] = disk_benchmark();
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


		delete[] runtime;

		if (op_type == RR) {
			for (int i = 0; i < thread_num; i++)
				delete[] rdmIndex[i];
			delete[] rdmIndex;
		}
		for (int i = 0; i < thread_num; i++)
			delete[] bufferStore[i];
		delete[] bufferStore;

		delete[] fileRangePerThrd;
		delete[] fileStartPerThrd;
		delete[] numOptPerThrd;

		return 0;

}


/**
 * disk benchmark implementation
 * @return running time in seconds
 */
double disk_benchmark () {
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
	} else if (op_type == SR) {
		for (int tid = 0; tid < thread_num; tid++)
			pthread_create(&threads[tid], NULL, sqtialRead, (void *)(thrdID + tid));
	} else if (op_type == RR) {
		for (int tid = 0; tid < thread_num; tid++)
			pthread_create(&threads[tid], NULL, rdmRead, (void *)(thrdID + tid));
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
 * sequential read and write benchmark
 * @param  argv thread ID
 * @return      [description]
 */
void *readWrite (void *argv) {
	int crtThrdID = *(int *)argv;
	for (size_t i = 0; i < numOptPerThrd[crtThrdID]; i++) {
		pread(readFile, bufferStore[crtThrdID], block_size, fileStartPerThrd[crtThrdID]+i*block_size);
		pwrite(writeFile, bufferStore[crtThrdID], block_size, fileStartPerThrd[crtThrdID]+i*block_size);
	}

	return NULL;
}


/**
 * sequential read access benchmark
 * @param  argv thread ID
 * @return      [description]
 */
void *sqtialRead (void *argv) {
	int crtThrdID = *(int *)argv;
	for (size_t i = 0; i < numOptPerThrd[crtThrdID]; i++) {
		pread(readFile, bufferStore[crtThrdID], block_size, fileStartPerThrd[crtThrdID]+i*block_size);
	}
	return NULL;
}

/**
 * random raed access benchmark
 * @param  argv  thread ID
 * @return      [description]
 */
void *rdmRead (void *argv) {
	int crtThrdID = *(int *)argv;
	for (size_t i = 0; i < numOptPerThrd[crtThrdID]; i++) {
		pread(readFile, bufferStore[crtThrdID], block_size, fileStartPerThrd[crtThrdID] + rdmIndex[crtThrdID][i]);
	}
	return NULL;

}





