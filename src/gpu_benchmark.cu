/**
 * @file gpu_benchmark.cu
 * benchmark for GPU
 * author: Linlin Chen
 * lchen96@hawk.iit.edu
 * 
 */

#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <sys/time.h>
#include <unistd.h>
#include <gpu_benchmark.h>

using namespace std;


/**
 * user manual, help user how to give input
 */
void helper (char *arg) {
	cout<<arg<<": GPU benchmark tool, Version 0.0.1 (lchen96@hawk.iit.edu)"<<endl<<endl;
	cout<<"usage:\t"<<arg<<" [-h] [-b] [-o <opearation>] [-l <option>] [-r <option>]"<<endl;
	cout<<"Arguments:"<<endl;
	cout<<"\t-h\tlist available commands"<<endl;
	cout<<"\t-b\ttest GPU bandwidth only"<<endl;
	cout<<"\t-o\toperation type, double=0, integer=1, half=2, quarter=3"<<endl;
	cout<<"\t-l\tnumber of loops[default = 8e^8]"<<endl;
	cout<<"\t-r\tnumber of repeated benchmark tests[default = 1]"<<endl;
	cout<<endl;
}






int main (int argc, char * argv[]) {
	/*
	use getopt function to parse the input arguments
	 */
	int c;
	int flag;
	// only accepts "hfitlr" arguments, where for "tlr" the argument can be optional
	while ((c = getopt (argc, argv, ":hbo::l::r::")) != -1) 
		switch (c){
			case 'h':
				helper();
				exit(0);
				break;
			case 'b':
				testBandwidth = true;
				break;
			case 'o':
				flag = stoi(optarg);
				if (flag == 0)
					op_type = FLOP;
				else if (flag == 1)
					op_type = IOP;
				else if	(flag == 2)
					op_type = HOP;
				else if (flag == 3)
					op_type = QOP;
				else {
					cerr<<"-o: option type can only be 0, 1, 2, 3!\n"<<endl;
					helper(argv[0]);
					exit(1);
				}
				break;
			case 'l':
				loop_num = (long) atof(optarg);
			case 'r':
				repeat_num = stoi(optarg);
			case ':':
				if (optopt == 'o')
					op_type = FLOP;
				else if (optopt == 'l') 
					loop_num = DEFAULTLOOP;
				else if (optopt == 'r') 
					repeat_num = 1;
				break;
			case '?':
			default:
				cerr<<"Invalid arguments!"<<endl;
				helper(argv[0]);
				abort();
		}

		//output user's setting information for benchmark
		cout<<"\nThe benchmarking begins with:"
			<<"\n\tOperation:\t\t"<<op[op_type]
			<<"\n\t#Loop:\t\t\t"<<loop_num
			<<"\n\t#Iteration:\t\t"<<repeat_num<<endl<<endl<<endl;


		int config[2] = {0};
		core_num = computeConfiguration(config);	//get the number of GPU cores

		float *runtime = new float[repeat_num];
		cout<<"GPU Info:\n\tGPU: "<<dev.name<<"\n\tCore Num: "<<core_num
		if (testBandwidth) {
			cout<<"GPU\tBlockSize\tDataSize\tBandwidth(MB/s)\tTime"<<endl;
		} else {
			if (op_type == FLOP)
				cout<<"GPU\tOptType\t#Operation\tGFLOPS\tTime"<<endl;
			else if (op_type == IOP)
				cout<<"GPU\tOptType\t#Operation\tGIOPS\tTime"<<endl;
			else if (op_type == HOP)
				cout<<"GPU\tOptType\t#Operation\tGHOPS\tTime"<<endl;
			else if (op_type == QOP)
				cout<<"GPU\tOptType\t#Operation\tGQOPS\tTime"<<endl;
		}
		for (int i = 0; i < repeat_num; i++) {
			//runtime[i] = gpu_benchmark();
			if (testBandwidth) {
				runtime[i] = gpu_bdwth_benchmark();

				cout<<"#Iter "<<i<<"\t"<<BYTE_IN_MB(BLOCKSIZE)<<"MB\t"
				<<BYTE_IN_MB(BLOCKSIZE*block_num)<<"MB\t"
				<<BYTE_IN_MB(BLOCKSIZE*block_num)/runtime[i]<<"MB/s\t"<<runtime[i]<<"s"<<endl;
			} else {
				runtime[i] = gpu_thrpt_benchmark();

				cout<<"#Iter "<<i<<"\t"<<op[op_type]<<"\t"<<loop_num<<"\t"
					<<(loop_num * core_num) / runtime[i] / 1e9<<"\t"<<runtime[i]<<"s"<<endl;
			}
		}


}


double gpu_bdwth_benchmark () {
	cudaEvent_t start, stop;
  	cudaEventCreate(&start);
  	cudaEventCreate(&stop);

  	
}

double gpu_thrpt_benchmark () {

}




//kernel function for double precision operation
__global__
void DFL_Ops() {


}

//kernel function for integer operation
__global__
void Int_Ops() {

}


//kernel function for half-precision
__global__
void H_Ops() {

}

//kernel function for quater-precision
__global__
void Q_Ops() {

}
