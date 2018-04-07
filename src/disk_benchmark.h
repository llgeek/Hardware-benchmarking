#ifndef _DISK_H_
#define _DISK_H_

#include <cstddef>
//#include <cstdio>
#include <string>


#define RDW 0	//read and write
#define SR 1	//Sequantial read
#define RR	2	//random raed

#define LTC 0	//latency
#define THRPT 1	//throughput

#define EBBLOCK	8L				//8b
#define EKBBLOCK	8192L		//8kb
#define EMBBLOCK 8388608L		//8MB
#define ETMBBLOCK 83886080L		//80MB

#define ONEKB	1024L			//1KB in Byte
#define ONEMB	1048576L		//1MB in Byte
#define ONEGB	1073741824L		//1GB in Byte

#define KB_IN_BYTE(KB) (KB*ONEKB)	//KB in Bytes
#define MB_IN_BYTE(MB) (MB*ONEMB)	//MB in Bytes
#define GB_IN_BYTE(GB) (GB*ONEGB)	//GB in Bytes

#define BYTE_IN_KB(B) ((double)B/ONEKB)	//Bytes in KB
#define BYTE_IN_MB(B) ((double)B/ONEMB)	//Bytes in MB
#define BYTE_IN_GB(B) ((double)B/ONEGB)	//Bytes in GB

#define DEFAULTDATASIZE GB_IN_BYTE(10L)		//10GB

typedef int OP_TYPE;		//operation type
typedef int METRIC;		//metric

const int MAXTHREADS = 20;
const long MINDATASIZE = GB_IN_BYTE(1L);
const long MAXBLOCKSIZE = MB_IN_BYTE(100L);

const char* op[] = {"Sequential Read&Write", "Sequential Read", "Random Read"};

const char* RDFILENAME = "toread.bin";
const char* WTFILENAME = "towrite.bin";

/*
global variables
 */
OP_TYPE op_type = RDW;
METRIC metric = THRPT;
bool skip_createfile = false;
int thread_num = 1;
long data_size = DEFAULTDATASIZE;
long block_size = EBBLOCK;
int repeat_num = 1;


std::size_t** rdmIndex;	//generate random variables for each thread, used in random write
char** bufferStore;		//buffer to store content read from file, each thread possessing one unique buffer
int writeFile;		//file descriptor to be written
int readFile;		//file descriptor to be read

std::size_t* fileRangePerThrd;	//file location range per thread, defines the range from start point each thread can access
std::size_t* fileStartPerThrd;	//file offset to the beginning for each thread
std::size_t* numOptPerThrd;		//number of operations per thread


/*
function declarations
 */
void helper (char *arg);
long getSizeInByte (std::string input);
double disk_benchmark ();
void *readWrite (void *argv);
void *sqtialRead (void *argv);
void *rdmRead (void *argv);




#endif