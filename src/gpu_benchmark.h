#ifndef _GPU_H_
#define _GPU_H_

#include <cstddef>
#include <string>

#define FLOP	0	//double precision float 64-bit
#define IOP	1	//integer precision 32-bit
#define HOP	2	//half precision 16-bit
#define QOP	3	//quater precision 8-bit

#define KB_IN_BYTE(KB) (KB*ONEKB)	//KB in Bytes
#define MB_IN_BYTE(MB) (MB*ONEMB)	//MB in Bytes
#define GB_IN_BYTE(GB) (GB*ONEGB)	//GB in Bytes

#define BYTE_IN_KB(B) ((double)B/ONEKB)	//Bytes in KB
#define BYTE_IN_MB(B) ((double)B/ONEMB)	//Bytes in MB
#define BYTE_IN_GB(B) ((double)B/ONEGB)	//Bytes in GB


#define DEFAULTLOOP 8e8	//default value for loop_num
#define BLOCKSIZE MB_IN_BYTE(80L)

typedef int OP_TYPE;

const char* op[] = {"DoubleFloat", "Integer", "HalfPrecision", "QuaterPrecision"};


/*
* global variabls
 */
bool testBandwidth = false;

OP_TYPE op_type = FLOP;		//opeartion type: float or integer, default = float
long loop_num = DEFAULTLOOP;		//loop num, default = 8e9
std::size_t block_num = 1000;		//number of blocks to be written in GPU
int repeat_num = 1;		//repeat test num, defeault = 1

int core_num = 0;


/*
functions declaration
 */




#endif