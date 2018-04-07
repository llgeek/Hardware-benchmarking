/**
 * @file cpu_benchmark.c
 * cpu benchmark 
 *
 * author: Linlin Chen
 * lchen96@hawk.iit.edu
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#include <string>
#include "cpu_benchmark.h"

using namespace std;



/**
 * user manual, help user how to give input
 */
void helper (char *arg) {
	cout<<arg<<": CPU benchmark tool, Version 0.0.1 (lchen96@hawk.iit.edu)"<<endl<<endl;
	cout<<"usage:\t"<<arg<<" [-h] [-o <operation>] [-t <threads>] [-l <loops>] [-r <repeats>]"<<endl;
	cout<<"Arguments:"<<endl;
	cout<<"\t-h\tlist available commands"<<endl;
	cout<<"\t-o\toperation type, DbFloat=0, Integer=1"<<endl;
//	cout<<"\t-f\ttest double precision floating point operations (defaulted)"<<endl;
//	cout<<"\t-i\ttest integer operatons"<<endl;
	cout<<"\t-t\tnumber of threads ( <= "<<MAXTHREADS<<") [default = 1]"<<endl;
	cout<<"\t-l\tnumber of loops[default = 1e^9]"<<endl;
	cout<<"\t-r\tnumber of repeated benchmark tests[default = 1]"<<endl;
	cout<<endl;
}



int main (int argc, char *argv[]) {
	/*
	use getopt function to parse the input arguments
	 */
	int c;
	// only accepts "hfitlr" arguments, where for "tlr" the argument can be optional
	while ((c = getopt (argc, argv, ":ho::t::l::r::")) != -1) 
		switch (c) {
			case 'h':	//helper
				helper(argv[0]);
				exit(0);
			case 'o':	//operation type
				if (stoi(optarg) == 0)
					op_type = FLOP;
				else if (stoi(optarg) == 1)
					op_type = IOP;
				else {
					cerr<<"option type can only be 0 or 1!\n"<<endl;
					helper(argv[0]);
					exit(1);
				}
				break;
			case 't':		//thread number
				if ((thread_num = atoi(optarg)) > MAXTHREADS) {
					cout<<"Exceed the supported thread limit! ( <= "<<MAXTHREADS<<")"<<endl;
					exit(2);
				}
				break;
			case 'l':		//loop number, equally is the number of total operations
				loop_num = (long)atof(optarg);	//accept exponent input like 8e9
				break;
			case 'r':	//times to repeat the benchmark 
				repeat_num = stoi(optarg);
				break;
			case ':':	/*missing option argument, using default value*/
				if (optopt == 'o') 
					op_type = FLOP;
				else if (optopt == 't') 
					thread_num = 1;
				else if (optopt == 'l')
					loop_num = DEFAULTLOOP;
				else if (optopt == 'r')
					repeat_num = 1;
				break;
			case '?':	/* invalid option*/
			default:
				cerr<<"Invalid arguments!"<<endl;
				helper(argv[0]);
				abort();
		}


		//output user's setting information for benchmark
		cout<<"\nThe benchmarking begins with:"
			<<"\n\tOperation:\t\t"<<op[op_type]
			<<"\n\t#Thread:\t\t"<<thread_num
			<<"\n\t#Loop:\t\t\t"<<loop_num
			<<"\n\t#Iteration:\t\t"<<repeat_num<<endl<<endl<<endl;



		//float *runtime = (float *) malloc (sizeof(float) * repeat_num); 
		float *runtime = new float[repeat_num];
		if (op_type == FLOP)
			cout<<"CPU\tOpType\t#Thread\t#Operation\tGFLOPS\tTime"<<endl;
		else
			cout<<"CPU\tOpType\t#Thread\t#Operation\tGIOPS\tTime"<<endl;

		for (int i = 0; i < repeat_num; i++) {
			runtime[i] = cpu_benchmark();
			cout<<"#Iter "<<i<<"\t"<<op[op_type] 
				<<"\t"<<thread_num<<"\t"<<loop_num<<"\t"<<(loop_num * 18 / runtime[i] / 1e9)<<"\t"<<runtime[i]<<endl;

		}
		delete[] runtime;
		return 0;

}




/**
 * Benchmark function for testing FLOPs or IOPs
 * multiple threads involved
 * @return benchmarking time in seconds
 */
float cpu_benchmark() {		//return running time in seconds
	struct timeval starttime, endtime;
	long runtime;
	//pthread_t *threads = (pthread_t *) malloc(sizeof(pthread_t) * thread_num);
	pthread_t *threads = new pthread_t [thread_num];
	if (op_type == FLOP) {		//float test
		gettimeofday(&starttime, NULL);
		for (int i = 0; i < thread_num; i++) {
			pthread_create(&threads[i], NULL, doubleOPs, NULL);
		}
		for (int i = 0; i < thread_num; i++) {
			pthread_join(threads[i], NULL);
		}
		gettimeofday(&endtime, NULL);
	} else if (op_type == IOP) {		//integer test
		gettimeofday(&starttime, NULL);
		for (int i = 0; i < thread_num; i++) {
			pthread_create(&threads[i], NULL, integerOPs, NULL);
		}
		for (int i = 0; i < thread_num; i++) {
			pthread_join(threads[i], NULL);
		}
		gettimeofday(&endtime, NULL);
	} else {
		cerr<<"Invalid structions! Only accept -f or -i!"<<endl;
		//helper();
		abort();
	}
	delete[] threads;
	runtime = (endtime.tv_sec - starttime.tv_sec) * 1000000L + (endtime.tv_usec - starttime.tv_usec);
	return runtime/1000000.0;	//second

}





/**
 * Float operation benchmark
 * @return [description]
 */
 void *doubleOPs(void *arg){
 	double factor1 = 2.0;
	double result1 = 0.0;

	double factor2 = 3.0;
	double result2 = 1.0;

	long loop = loop_num/thread_num;
	// one incrementation and one comparasion in loop, one float add
	for (size_t i = 0; i < loop; i++) {
		result1 = factor1 * 2.3 + 312.2;
		result1 = factor1 * 3.1 + 221;
		result1 = factor2 * 3.45 + 312;
		result2 = factor1 * 1.2 - 2.0;
		result2 = factor2 * 2.1 + 3.4;
		result2 = factor1 * factor2 - 3.523;
	}
	return NULL;
}



/**
 * Integer operation benchmark
 * @return [description]
 */
void *integerOPs(void *arg) {
	 size_t factor1 = 2;
	size_t result1 = 1;

	size_t factor2 = 3;
	size_t result2 = 4;

	long loop = loop_num/thread_num;
	// one incrementation and one comparasion in loop, one interger add
	for (size_t i = 0; i < loop; i++) {
		result1 = factor1 * 2 + 312;
		result1 = factor1 * 3 + 221;
		result1 = factor2 * 3 + 312;
		result2 = factor1 * 1 - 2;
		result2 = factor2 * 2 + 3;
		result2 = factor1 * factor2 - 3;
	}
	return NULL;
}

