/*
    C ECHO client example using sockets
*/
#include <stdio.h> //printf
#include <string.h>    //strlen
#include <sys/socket.h>    //socket
#include <arpa/inet.h> //inet_addr
#define MAX_LENGTH 512
/*
	arg1 : IP
	arg2 : port
	arg3 : procedure no
	arg4 : param1
	arg5 : param2
*/
int bindNewSocket(const char* ipAddress, int message, int procedure_no, int param1, int param2);

int main(int argc, char const *argv[])
{
	int sock;
	int param1, param2;
	int procedure_no;
	char message[MAX_LENGTH];

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

    printf("port no: %s\n", message);

    bindNewSocket(ipAddress, atoi(message), procedure_no, param1, param2);

    close(sock);

	return 0;
}

int bindNewSocket(const char* ipAddress, int newPort, int procedure_no, int param1, int param2){
	int sock;
	struct sockaddr_in server;
	struct hostent *check_server;

	server.sin_addr.s_addr = inet_addr(ipAddress);
    server.sin_family = AF_INET;
    server.sin_port = htons( newPort );

    //Socket creation
	if( (sock = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
	{
		perror(":socket failed");
		return -1;
	}

    printf("ip: %s\n", ipAddress);
    printf("newPort: %d\n", newPort);

	//printf("trying to connect...\n");
	if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror(":connect failed");
        close(sock);
        return -1;
    }

    puts("Connected\n");
    
    printf("client exited\n");

    close(sock);

    return 0;

}