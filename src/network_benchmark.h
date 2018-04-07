#ifndef _NETWORK_H_
#define _NETWORK_H_

#include <cstddef>
//#include <cstdio>
#include <string>

#define TCP 0
#define UDP 1

#define SVR 0	//server
#define CLT 1	//client

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

#define DEFAULTDATASIZE GB_IN_BYTE(8L)		//8GB

#define BUFFERSIZE KB_IN_BYTE(64L)

#define SERVERBASEPORT 8888		//base port for server, each thread increments its own port

typedef int OP_TYPE;		//operation type
typedef int METRIC;		//metric
typedef int ROLE;	//role, client or server

const int MAXTHREADS = 20;
const long MINDATASIZE = GB_IN_BYTE(1L);

const char* op[] =  {"TCP", "UDP"};

const char LOCALHOST[] = "127.0.0.1";

/*
global variables
 */

int thread_num = 1;
OP_TYPE op_type = TCP;
int repeat_num = 1;

ROLE role = SVR;

char serverIP[255] = "127.0.0.1"; 
std::size_t data_size = GB_IN_BYTE(8L);		//transfer 8GB data


char **recBuffer;		//receive buffer for each thread, buffer size equals 64KB
char **sendBuffer;		//sender buffer for each thread, to store the data to be sent, equal 64KB




/*
function declaration
 */


void helper (char *arg);
double network_benchmark ();
void *serverTCP (void *argv);
void *serverUDP (void *argv);
void *clientTCP (void *argv);
void* clientUDP(void *argv);


#endif