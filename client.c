/*
    C ECHO client example using sockets
*/
#include <stdio.h> //printf
#include <string.h>    //strlen
#include <sys/socket.h>    //socket
#include <arpa/inet.h> //inet_addr
#include <fcntl.h>
#define MAX_LENGTH 512
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

int main(int argc, char const *argv[])
{
	int sock;
	int param1, param2;
	int procedure_no;
	char message[MAX_LENGTH];
	double res = 0;
	int flags;

	if(argc < 6){
		printf("invalid argumants\n");
		return -1;
	}
	
	const char* ipAddress = argv[1];
	const int portNo = atoi(argv[2]);
	struct sockaddr_in server;


	procedure_no = atoi(argv[3]);
	param1 = atoi(argv[4]);
	param2 = atoi(argv[5]);

	//Socket creation
	if( (sock = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
	{
		perror(":socket failed");
		return -1;
	}

	flags = fcntl(sock, F_GETFL, 0);
	if (flags < 0){
		perror("fcntl failed");
		return -1;
	} 
		
	flags = (flags&~O_NONBLOCK);
	
	fcntl(sock, F_SETFL, flags);

	server.sin_addr.s_addr = inet_addr(ipAddress);
    server.sin_family = AF_INET;
    server.sin_port = htons( portNo );

    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        return 1;
    }
     
    puts("Connected\n");

    memset(message, 0 , sizeof(message));
    strcpy(message, argv[3]);

    //Send some data
    if( send(sock , message , strlen(message) , 0) < 0)
    {
        perror("send failed");
        close(sock);
        return -1;
    }

    printf("send\n");

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

    sock = bindNewSocket(ipAddress, atoi(message), procedure_no, param1, param2);

    sendParams(sock, param1, param2);
    res = getResult(sock);

    printf("res: %.4lf\n", res);

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

    printf("ip: %s\n", ipAddress);
    printf("newPort: %d\n", newPort);

	//printf("trying to connect...\n");
	if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed:");
        close(sock);
        return -1;
    }

    puts("Connected to thread\n");

    //printf("client exited\n");

    return sock;
}

void sendParams(int sock, double param1, double param2){
	char sending[MAX_LENGTH];

	memset(sending, 0, MAX_LENGTH);
    sprintf(sending, "%.2lf", param1);
    printf("param1: %s\n", sending);

    if( send(sock , sending , sizeof(sending) , 0) < 0){
        perror("send failed");
        close(sock);
        return;
    }

    memset(sending, 0, sizeof(sending));
    sprintf(sending, "%.2lf", param2);
    printf("param2: %s\n", sending);

    if( send(sock , sending , sizeof(sending) , 0) < 0){
        perror("send failed");
        close(sock);
        return ;
    }
    
}

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

    printf("receive: %s\n", receive);

    sscanf(receive, "%lf", &result);
    //result = atof("100");

    return result;
}