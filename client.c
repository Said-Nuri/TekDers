/*
    C ECHO client example using sockets
*/
#include <stdio.h> //printf
#include <string.h>    //strlen
#include <sys/socket.h>    //socket
#include <arpa/inet.h> //inet_addr
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#define MAX_LENGTH 512
#define MICRO_SEC 1000

//#define DEBUG 


/*
	arg1 : IP
	arg2 : port
	arg3 : procedure no
	arg4 : param1
	arg5 : param2
*/
int bindNewSocket(const char* ipAddress, int message, int procedure_no, double param1, double param2);
void sendParams(int sock, double param1, double param2);
double getResult(int sock);
int getThreadID(int sock);
void printOp(int opno);

int main(int argc, char const *argv[])
{
	clock_t start, end;
	double dif;
	int sock;
	double param1, param2;
	int procedure_no;
	char message[MAX_LENGTH];
	double res = 0;
	int flags;
	int thread_id;

	if(argc < 6){
		printf("invalid argumants\n");
		return -1;
	}
	
	start = clock();

	const char* ipAddress = argv[1];
	const int portNo = atoi(argv[2]);
	struct sockaddr_in server;


	procedure_no = atoi(argv[3]);
	

	sscanf(argv[4], "%lf", &param1);
	sscanf(argv[5], "%lf", &param2);

	//Socket creation
	if( (sock = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
	{
		perror("socket failed");
		puts("client exited");
		
	}
	
	if(sock == -1)
		return -1;


	flags = fcntl(sock, F_GETFL, 0);
	if (flags < 0){
		perror("fcntl failed");
		puts("client exited");
		exit(EXIT_FAILURE);
	} 
		
	flags = (flags&~O_NONBLOCK);
	
	fcntl(sock, F_SETFL, flags);

	server.sin_addr.s_addr = inet_addr(ipAddress);
    server.sin_family = AF_INET;
    server.sin_port = htons( portNo );

    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed");
        puts("client exited");
        close(sock);
        exit(EXIT_FAILURE);
    }
    
    #ifdef DEBUG
    puts("Connected\n");
    #endif

    memset(message, 0 , sizeof(message));
    strcpy(message, argv[3]);

    //Send op code
    if( send(sock , message , strlen(message) , 0) < 0)
    {
        perror("send failed");
        puts("client exited");
        close(sock);
        return -1;
    }

    #ifdef DEBUG
    printf("send\n");
    #endif

    memset(message, 0 , sizeof(message));

    //Receive a port number from the server
    if( recv(sock , message , sizeof(message) , 0) < 0)
    {
        perror("recv failed");
        close(sock);
        return -1;
    }

    close(sock);
    
    //printf("port no: %s\n", message);

    if(atoi(message) == -1)
    	return -1;

    sock = bindNewSocket(ipAddress, atoi(message), procedure_no, param1, param2);

    end = clock();
    dif = end - start;
    
    if(procedure_no == 4){
    	param1 = 0;
    	param2 = dif/MICRO_SEC;
    	
    	#ifdef DEBUG	
    	printf("dif: %lf\n", dif);
    	#endif
    }

    sendParams(sock, param1, param2);
    res = getResult(sock);
    thread_id = getThreadID(sock);

    printf("parameter 1: %lf\n", param1);
    printf("parameter 2: %lf\n", param2);
    printOp(procedure_no);
    printf("Result: %.6lf\n", res);
    printf("Thread id: %u\n", thread_id);
    printf("Process ID : %d\n", (int)getpid());

    end = clock();
    dif = end - start;
    printf("Elapsed time in millisecond: %0.6lf\n", dif/MICRO_SEC);

    close(sock);

	return 0;
}

int bindNewSocket(const char* ipAddress, int newPort, int procedure_no, double param1, double param2){
	int sock;
	struct sockaddr_in server;

	server.sin_addr.s_addr = inet_addr(ipAddress);
    server.sin_family = AF_INET;
    server.sin_port = htons( newPort );

    //Socket creation
	if( (sock = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
	{
		perror("socket failed:");
		return -1;
	}

	#ifdef DEBUG
    printf("ip: %s\n", ipAddress);
    printf("newPort: %d\n", newPort);
    #endif

	//printf("trying to connect...\n");
	if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed:");
        close(sock);
        return -1;
    }

    #ifdef DEBUG
    puts("Connected to thread\n");
    #endif

    //printf("client exited\n");

    return sock;
}

void sendParams(int sock, double param1, double param2){
	char sending[MAX_LENGTH];

	memset(sending, 0, MAX_LENGTH);
    sprintf(sending, "%.2lf", param1);
    
    #ifdef DEBUG
    printf("param1: %s\n", sending);
    #endif

    if( send(sock , sending , sizeof(sending) , 0) < 0){
        perror("send failed");
        close(sock);
        return;
    }

    memset(sending, 0, sizeof(sending));
    sprintf(sending, "%.2lf", param2);
    
    #ifdef DEBUG
    printf("param2: %s\n", sending);
    #endif

    if( send(sock , sending , sizeof(sending) , 0) < 0){
        perror("send failed");
        close(sock);
        return ;
    }
    
}

/********** Send Result ************/
double getResult(int sock){
	char receive[MAX_LENGTH];
	double result;

	memset(receive, 0, MAX_LENGTH);

	//Receive a port number from the server
    if( recv(sock , receive , MAX_LENGTH , 0) < 0)
    {
        perror("recv failed");
        close(sock);
        return -1;
    }

    #ifdef DEBUG
    printf("receive: %s\n", receive);
    #endif

    sscanf(receive, "%lf", &result);
    //result = atof("100");

    return result;
}

int getThreadID(int sock){
	char receive[MAX_LENGTH];
	double id;

	memset(receive, 0, MAX_LENGTH);

	//Receive thread id
    if( recv(sock , receive , MAX_LENGTH , 0) < 0)
    {
        perror("recv failed");
        close(sock);
        return -1;
    }

    return atoi(receive);
}

void printOp(int opno){
	
	printf("Operation name: ");
	switch(opno){
		case 1:
			printf("Square Root\n");
			break;
		case 2:
			printf("Sinus\n");
			break;
		case 3:
			printf("Absolute\n");
			break;
		case 4:
			printf("Integral\n");
			break;
		default:
			printf("invalid Operation!\n");
			break;
	}
}