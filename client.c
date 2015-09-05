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
        puts("Send failed");
        return -1;
    }

    close(sock);

	return 0;
}