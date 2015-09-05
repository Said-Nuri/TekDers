#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h> 
#define MAX_LENGTH 512

void *operation_thread(void *socket_desc);

/*
	arg1 : IP
	arg2 : port
	arg3 : Number of thread per server
*/
int main(int argc, char const *argv[])
{
	int i, c;
	const char* ipAddress = argv[1];
	const int portNo = atoi(argv[2]);
	int numofthread = atoi(argv[3]);
	pthread_t op_threads[numofthread];
	int socketFd;
	int optval = 1;
	int port_sent;
	char opno;
	char recieve[MAX_LENGTH];
	int op_check[4] = {0, 0, 0, 0};
	int client_sock[numofthread];
	struct sockaddr_in server, client;
	int counter = 0;

	if(argc < 4){
		printf("invalid argumants\n");
		return -1;
	}

	//Socket creation
	if( (socketFd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
	{
		perror(":socket failed");
		return -1;
	}

	puts("Socket created");

	//Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(ipAddress);
    server.sin_port = htons( portNo );

    //Bind
    if( bind(socketFd,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return -1;
    }
    puts("bind done");

    //Listen
    listen(socketFd , 3);

    c = sizeof(struct sockaddr_in);

    while( (client_sock[counter] = accept(socketFd, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {	
        puts("Connection accepted");
        
        memset(recieve, 0 , sizeof(recieve));

        //Receive a reply from the server
        if( recv(client_sock[counter] , recieve , MAX_LENGTH , 0) < 0){
            perror("recv failed");
            break;
        }

        printf("op no: %s\n", recieve);

        port_sent = check_operation(recieve);

        //Create thread
        if( pthread_create( &op_threads[counter] , NULL ,  operation_thread , (void*) &client_sock[counter]) < 0)
        {
            perror("could not create thread");
            return 1;
        }

        pthread_join(op_threads[counter], NULL);

        counter++;

        if(counter == 2)
        	break;
    }

    if (client_sock[counter] < 0){
		perror("accept failed");	
	}

    close(socketFd);

	return 0;
}

void *operation_thread(void *socket_desc){

	//Get the socket descriptor
    int sock = *(int*)socket_desc;


	printf("socket desc: %d\n", *(int*)socket_desc);
}