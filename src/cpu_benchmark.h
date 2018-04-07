#ifndef _CPU_HEADER_
#define _CPU_HEADER_

#define FLOP 0
#define IOP 1

typedef int OP_TYPE;


const int MAXTHREADS = 20;
const long DEFAULTLOOP = 8e8;

const char* op[] = {"DoubleFloat", "Integer"};

/*
* global variables
 */
OP_TYPE op_type = FLOP;		//opeartion type: float or integer, default = float
int thread_num = 1;		//thread num, default = 1
long loop_num = DEFAULTLOOP;		//loop num, default = 8e9
int repeat_num = 1;		//repeat test num, defeault = 1



/*
* functions
 */
void helper (char *arg);
float cpu_benchmark ();
void *doubleOPs (void *arg);
void *integerOPs (void *arg);



#endif
