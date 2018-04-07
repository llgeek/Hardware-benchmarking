/**
 * @File: network_benchmark.c
 *benchmark the network communication latency and throughput
 *
 *author: Linlin Chen
 *lchen96@hawk.iit.edu
 * 
 */


#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <ifaddrs.h>

#include <string>
#include <pthread.h>

#include "network_benchmark.h"

using namespace std;


/**
 * user manual, help user how to give input
 */
void helper (char *arg) {
	cout<<arg<<": Network benchmark tool, Version 0.0.1 (lchen96@hawk.iit.edu)"<<endl<<endl;
	cout<<"usage:\t"<<arg<<" [-h] [-f <role>] [-p <protocol>] [-a <address>] [-t <threads>] [-r <repeats>]"<<endl;
	cout<<"Arguments:"<<endl;
	cout<<"\t-h\tlist available commands"<<endl;
	cout<<"\t-f\trole of the App, server=0, client=1"<<endl;
	cout<<"\t-p\tprotocol, TCP=0, UDP=1"<<endl;
	cout<<"\t-a\tserver address (default 127.0.0.1)"<<endl;
	cout<<"\t-t\tnumber of threads ( <= "<<MAXTHREADS<<") [default = 1]"<<endl;
	cout<<"\t-r\tnumber of repeated benchmark tests[default = 1]"<<endl;
	cout<<endl;

}



int main (int argc, char* argv[]) {
	/*
	use getopt function to parse the input arguments
	 */
	int c;
	int flag;
	while ((c = getopt (argc, argv, ":hf::p::a::t::r::")) != -1) 
		switch (c) {
			case 'h':
				helper(argv[0]);
				exit(0);
				break;
			case 'f':
				flag = stoi(optarg);
				if (flag == 0) 
					role = SVR;
				else if (flag == 1)
					role = CLT;
				else {
					cerr<<"option type can only be 0 or 1!\n"<<endl;
					helper(argv[0]);
					exit(1);
				}
				break;
			case 'p':
				flag = stoi(optarg);
				if (flag == 0) 
					op_type = TCP;
				else if (flag == 1)
					op_type = UDP;
				else {
					cerr<<"option type can only be 0 or 1!\n"<<endl;
					helper(argv[0]);
					exit(1);
				}
				break;
			case 'a':
				strcpy(serverIP, optarg);
				break;
			case 't':
				if ((thread_num = atoi(optarg)) > MAXTHREADS) {
					cout<<"Exceed the supported thread limit! ( <= "<<MAXTHREADS<<")"<<endl;
					exit(2);
				}
				break;
			case 'r':
				repeat_num = stoi(optarg);
				break;
			case ':':
				if (optopt == 'p')
					op_type = TCP;
				else if (optopt == 'a')
					strcpy(serverIP, LOCALHOST);
				else if (optopt == 't')
					thread_num = 1;
				else if (optopt == 'r')
					repeat_num = 1;
				break;
			case '?':	//invalid argument
			default:
				cerr<<"Invalid arguments!\n"<<endl;
				helper(argv[0]);
				abort();
		}

		/*
		output user's input information
		 */	
		cout<<"\nThe benchmarking begins with:"
			<<"\n\tProtcol:\t\t"<<op[op_type]
			<<"\n\t#Thread:\t\t"<<thread_num
			<<"\n\tData size:\t\t"<<BYTE_IN_GB(data_size)<<" GB"
			<<"\n\tBuffer size:\t\t"<<BYTE_IN_KB(BUFFERSIZE)<<" KB"
			<<"\n\t#Iteration:\t\t"<<repeat_num<<endl<<endl<<endl;	



		recBuffer = new char* [thread_num];
		sendBuffer = new char* [thread_num];
		for (int i = 0; i < thread_num; i++) {
			recBuffer[i] = new char[BUFFERSIZE];
			sendBuffer[i] = new char[BUFFERSIZE];
			memset(sendBuffer[i], '1', BUFFERSIZE);
		}

		float *runtime = new float[repeat_num];

		cout<<"Network\tProctolType\t#Thread\tDataSize\tBufferSize\tThroughput(Mb/sec)\tLatency(us)"<<endl;


		//benchmark result
		for (int i = 0; i < repeat_num; i++) {
		runtime[i] = network_benchmark();
		cout<<"#Iter "<<i<<"\t"<<op[op_type]<<"\t"<<thread_num<<"\t"
			<<BYTE_IN_GB(data_size)<<"GB\t"<<BYTE_IN_KB(BUFFERSIZE)<<"KB\t"
			<<(BYTE_IN_MB(data_size)*8 / runtime[i])<<"\t"
			<<runtime[i]*1e6/data_size*8*1024<<"us"<<endl;
		}
}



/**
 * network benchmark implementation
 * @return running time in second
 */
double network_benchmark () {
	struct timeval starttime, endtime;
	long runtime;
	pthread_t *clientthreads = new pthread_t [thread_num];
	pthread_t *serverthreads = new pthread_t [thread_num];

	int* clientthrdID = new int[thread_num];
	int* serverthrdID = new int[thread_num];
	for (int i = 0; i < thread_num; i++) {
		clientthrdID[i] = i; 
		serverthrdID[i] = i;
	}

	gettimeofday(&starttime, NULL);
	if (op_type == TCP) {
		if (role == SVR) 		
			for (int tid = 0; tid < thread_num; tid++)
				pthread_create(&serverthreads[tid], NULL, serverTCP, (void *)(serverthrdID + tid));
		else	//this application is used as client
			for (int tid = 0; tid < thread_num; tid++)
				pthread_create(&clientthreads[tid], NULL, clientTCP, (void *)(clientthrdID + tid));

	} else if (op_type == UDP) {
		if (role == SVR)
			for (int tid = 0; tid < thread_num; tid++)
				pthread_create(&serverthreads[tid], NULL, serverUDP, (void *)(serverthrdID + tid));
		else	//this application is used as client
			for (int tid = 0; tid < thread_num; tid++)
				pthread_create(&clientthreads[tid], NULL, clientUDP, (void *)(clientthrdID + tid));
		
	}  else {
		cerr<<"Invalid structions! opType can only be 0,1,2!"<<endl;
		abort();
	}
	if (role == SVR)		//this application is used as server	
		for (int tid = 0; tid < thread_num; tid++) {
			pthread_join(serverthreads[tid], NULL);
		}
	else	//this application is used as client
		for (int tid = 0; tid < thread_num; tid++) {
			pthread_join(clientthreads[tid], NULL);
		}
	gettimeofday(&endtime, NULL);

	delete[] serverthreads;
	delete[] clientthreads;

	delete[] serverthrdID;
	delete[] clientthrdID;

	runtime = (endtime.tv_sec - starttime.tv_sec) * 1000000L + (endtime.tv_usec - starttime.tv_usec);	//time in microsecond
	return runtime/1000000.0;	//time in seconds 
}


/**
 * TCP handling used for server
 * @param  argv thread ID
 * @return      NULL
 */
void *serverTCP (void *argv) {
	int crtThrdID = *(int *)argv;
	int ret;
	
	struct sockaddr_in serverAddr;
	int tcpsocket;


	//create the TCP socket in server
	if ((tcpsocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("server: socket creation");
		exit(errno);
	}

	memset(&serverAddr, 0, sizeof(sockaddr_in));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVERBASEPORT + crtThrdID);	//thread i's port number will be 8888+i
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	//bind the socket
	if ((::bind(tcpsocket, (struct  sockaddr*) &serverAddr, sizeof(serverAddr))) == -1) {
		perror("server: socket bind");
		exit(errno);
	}

	//listen on the port
	if (listen(tcpsocket, SOMAXCONN) == -1) {
		perror("server: socket listen");
		exit(errno);
	}

	//accept any request
	int inreq;
	int addrlen = sizeof(serverAddr);
	while((inreq = accept(tcpsocket, (struct sockaddr *)&serverAddr, (socklen_t *)&addrlen))) {
	 	int read_size;
	 	while ((read_size = recv(inreq, recBuffer[crtThrdID], BUFFERSIZE, 0)) > 0);
	 	if (read_size == 0) {
	 		fflush(stdout);
	 	} else if (read_size == -1) {
	 		perror("server: receive data");
	 	}

	}
	if (inreq < 0) {
		perror("server: socket accept");
	 	exit(1); 
	}
	close(tcpsocket);
	pthread_exit(&ret);
}



/**
 * UDP handling used for server
 * @param  argv thread ID
 * @return      NULL
 */
void *serverUDP (void *argv) {
	int crtThrdID = *(int *)argv;
	int ret;
	
	struct sockaddr_in serverAddr;
	int udpsocket;

	if ((udpsocket = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("server: socket creation");
		exit(errno);
	}
	memset(&serverAddr, 0, sizeof(sockaddr_in));

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVERBASEPORT + crtThrdID);	//thread i's port number will be 8888+i
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	//bind the socket
	if (::bind(udpsocket, (struct  sockaddr*) &serverAddr, sizeof(serverAddr)) == -1) {
		perror("server: socket bind");
		exit(errno);
	}

	//receive data
	struct sockaddr_in clientAddr;
	socklen_t addrlen = sizeof(clientAddr);
	for (int i = 0; i < data_size / thread_num / BUFFERSIZE*2; i++) {
		if ((recvfrom(udpsocket, recBuffer[crtThrdID], BUFFERSIZE/2, 0, (struct sockaddr *)&clientAddr, &addrlen)) == -1) {
			perror("server: recvfrom");
		}
	}
	close(udpsocket);

	pthread_exit(&ret);
}



/**
 * TCP handling used for client
 * @param  argv thread ID
 * @return      [NULL
 */
void *clientTCP (void *argv) {
	int crtThrdID = *(int *)argv;
	int ret;

	struct sockaddr_in host_socket;
	int clientsock;

	//create socket
	if ((clientsock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("client: socket creation");
		exit(errno);
	}

	memset(&host_socket, '\0', sizeof(sockaddr_in));
	host_socket.sin_family = AF_INET;
	host_socket.sin_port = htons(SERVERBASEPORT + crtThrdID);
	if (inet_aton(serverIP, &host_socket.sin_addr) == 0) {
		perror("client: inet_aton");
		exit(errno);
	}

	//build the socket with server at desired port
	if (connect(clientsock, (struct sockaddr *) &host_socket, sizeof(host_socket)) < 0) {
		perror("client: connect");
		exit(errno);
	}

	//send data to the server
	for (int i = 0; i < data_size / thread_num / BUFFERSIZE; i++) {
		if (send(clientsock, sendBuffer[crtThrdID], BUFFERSIZE, 0) < 0) {
			perror("client: send");
			exit(errno);
		}
	}

	close (clientsock);

	pthread_exit(&ret);
}



/**
 * UDP handling used for client
 * @param argv [thread ID]
 */
void* clientUDP(void *argv) {
	int crtThrdID = *(int *)argv;
	int ret;

	struct sockaddr_in host_socket;
	int clientsock;

	//create socket
	if ((clientsock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		perror("client: socket creation");
		exit(errno);
	}

	memset(&host_socket, '\0', sizeof(sockaddr_in));
	host_socket.sin_family = AF_INET;
	host_socket.sin_port = htons(SERVERBASEPORT + crtThrdID);

	if (inet_aton(serverIP, &host_socket.sin_addr) == 0) {
		perror("client: inet_aton");
		exit(errno);
	}

	//send data through UDP to server
	//note UDP can only support maximum 2^16-1= 65535 Bytes
	for (int i = 0; i < data_size / thread_num / BUFFERSIZE*2 ; i++) {
		if ((sendto(clientsock, sendBuffer[crtThrdID], BUFFERSIZE/2, 0, (struct sockaddr *)&host_socket, sizeof(host_socket))) == -1) {
			perror("client: send");
			exit(errno);
		}
	}

	close(clientsock);

	pthread_exit(&ret);
}

