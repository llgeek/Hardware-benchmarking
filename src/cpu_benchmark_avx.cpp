/**
 * @file cpu_benchmark_avx.c
 * cpu benchmark 
 * AVX support
 *
 * author: Linlin Chen
 * lchen96@hawk.iit.edu
 */

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#include <string>
#include <cstring>
#include <immintrin.h>
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
				repeat_num = atoi(optarg);
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
			cout<<"#Iter "<<i<<"\t"<<(op_type == FLOP ? "Float":"Integer") 
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
 	__m256d factor1 = _mm256_set_pd(2.0, 4.4, 6.5, 8.9);
 	__m256d factor2 = _mm256_set_pd(1.0, 3.1, 4.7, 5.2);

 	__m256d factor3 = _mm256_set_pd(2.0, 4.4, 4.3, 8.9);
 	__m256d factor4 = _mm256_set_pd(1.0, 3.1, 5.7, 7.2);

 	__m256d result1;
 	__m256d result2;

	long loop = loop_num/thread_num;
	// one incrementation and one comparasion in loop, one float add
	for (size_t i = 0; i < loop; i++) {
		result1 = _mm256_mul_pd(factor1, factor2);
		result2 = _mm256_add_pd(factor3, factor4);

	}
	return NULL;
}



/**
 * Integer operation benchmark
 * @return [description]
 */
void *integerOPs(void *arg) {
	__m128i factor1 = _mm_set_epi32(2, 4, 6, 8);
 	__m128i factor2 = _mm_set_epi32(6, 3, 4, 5);

 	__m128i factor3 = _mm_set_epi32(2, 4, 4, 8);
 	__m128i factor4 = _mm_set_epi32(8, 3, 5, 7);

 	__m128i result1;
 	__m128i result2;
 	
	long loop = loop_num/thread_num;
	// one incrementation and one comparasion in loop, one interger add
	for (size_t i = 0; i < loop; i++) {
		result1 = _mm_add_epi32(factor1, factor2);
		result2 = _mm_mul_epi32(factor3, factor4);
	}
	return NULL;
}


