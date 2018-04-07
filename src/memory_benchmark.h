#ifndef _MEMORY_H_
#define _MEMORY_H_

#include <cstddef>
#include <string>


#define RDW	0	//read and write
#define SW 1	//sequential write
#define RMW 2	//random write

#define LTC 0	//latency metric
#define THRPT 1	//throughput metric

#define EBBLOCK	8L			//8b
#define EKBBLOCK	8192L		//8kb
#define EMBBLOCK 8388608L		//8MB
#define ETMBBLOCK 83886080L		//80MB

#define ONEKB	1024L		//1KB in Byte
#define ONEMB	1048576L		//1MB in Byte
#define ONEGB	1073741824L	//1GB in Byte

#define KB_IN_BYTE(KB) (KB*ONEKB)	//KB in Bytes
#define MB_IN_BYTE(MB) (MB*ONEMB)	//MB in Bytes
#define GB_IN_BYTE(GB) (GB*ONEGB)	//GB in Bytes

#define BYTE_IN_KB(B) ((double)B/ONEKB)	//Bytes in KB
#define BYTE_IN_MB(B) ((double)B/ONEMB)	//Bytes in MB
#define BYTE_IN_GB(B) ((double)B/ONEGB)	//Bytes in GB

#define DEFAULTDATASIZE MB_IN_BYTE(1280L)		//1280MB

typedef int OP_TYPE;		//operation type
typedef int METRIC;		//metric



const int MAXTHREADS = 20;
const long MINDATASIZE = GB_IN_BYTE(1L);
const long MAXBLOCKSIZE = MB_IN_BYTE(100L);

const char* op[] = {"Sequential Read&Write", "Sequantial Write", "Random Write"};

/* 
global variables
*/
OP_TYPE op_type = RDW;
METRIC metric = THRPT;
int thread_num = 1;
long data_size = DEFAULTDATASIZE;
long block_size = EBBLOCK;
int repeat_num = 1;


std::size_t** rdmIndex;	//generate random variables for each thread, used in random write
//char* dataSample;	//sample data, for write benchmark purpose
char* sourceMem;	//source memory, used in read+write
char* destMem;		//destination memory, for write benchmark purpose

std::size_t* memRangePerThrd;	//memory range per thread, defines the range from start point each thread can access
std::size_t* memStartPerThrd;	//memory offset to the beginning for each thread
std::size_t* numOptPerThrd;		//number of operations per thread


/*
functions declarations
 */
void helper (char *arg);
long getSizeInByte (std::string input);

double memory_benchmark ();
void *readWrite (void *argv);
void *sqtialWrite (void *argv);
void *rdmWrite (void *argv);


#endif