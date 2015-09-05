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
int check_operation(int op);

int op_check[4] = {0, 0, 0, 0};
static char global_ip[MAX_LENGTH];
/*
    arg1 : IP
    arg2 : port
    arg3 : Number of thread per server
*/
int main(int argc, char const *argv[])
{
    int i, c;
    const char* ipAddress = argv[1];
    int portNo = atoi(argv[2]);
    int numofthread = atoi(argv[3]);
    pthread_t op_threads[numofthread];
    int socketFd;
    int optval = 1;
    int port_sent;
    char opno;
    char recieve[MAX_LENGTH];
    char message[MAX_LENGTH];
    int client_sock[numofthread];
    struct sockaddr_in server, client;
    int counter = 0;

    if(argc < 4){
        printf("invalid argumants\n");
        return -1;
    }

    strcpy(global_ip, ipAddress);

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
        puts("New client accepted");
        
        memset(recieve, 0 , sizeof(recieve));

        //Receive a reply from the server
        if( recv(client_sock[counter] , recieve , MAX_LENGTH , 0) < 0){
            perror("recv failed");
            break;
        }

        //printf("op no: %s\n", recieve);

        // check op code
        port_sent = check_operation(atoi(recieve));

        //printf("port_sent %d\n", port_sent);
        
        if(port_sent != 0){
            
            memset(message, 0 , sizeof(message));
            sprintf(message, "%d", op_check[atoi(recieve)]);

            //Send some data
            if( send(client_sock[counter] , message , strlen(message) , 0) < 0){
                puts("Send failed");
                return 1;
            }
        }else{

            op_check[atoi(recieve)] = ++portNo;

            //Create thread
            if( pthread_create( &op_threads[counter] , NULL ,  operation_thread , (void*) &portNo) < 0)
            {
                perror("could not create thread");
                return 1;
            }

            sprintf(message, "%d", portNo);

            //Send some data
            if( send(client_sock[counter] , message , strlen(message) , 0) < 0){
                puts("Send failed");
                return 1;
            }

            //pthread_join(op_threads[counter], NULL);
        }

        /*
        //Create thread
        if( pthread_create( &op_threads[counter] , NULL ,  operation_thread , (void*) &client_sock[counter]) < 0)
        {
            perror("could not create thread");
            return 1;
        }

        pthread_join(op_threads[counter], NULL);
        */

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

void *operation_thread(void *portNo){
    int client_number = 2;
    int port = *(int*)portNo;
    int c;
    struct sockaddr_in server, client;
    int socketFd;
    int counter = 0;
    int client_sock[client_number];

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(global_ip);
    server.sin_port = htons( port );

    printf("thread started\n");
    //printf("global_ip: %s\n", global_ip);
    //printf("port: %d\n", port);

    if( (socketFd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
    {
        perror(":socket failed");
        close(socketFd);
        return;
    }

    //Bind
    if( bind(socketFd,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        close(socketFd);
        return ;
    }

     //Listen
    if( listen(socketFd , 3) < 0){
        perror("listen failed");
        close(socketFd);
        return ;
    }

    c = sizeof(struct sockaddr_in);

    printf("listening...\n");

    while( (client_sock[counter] = accept(socketFd, (struct sockaddr *)&client, (socklen_t*)&c)) ){
        puts("Connection accepted");

        printf("thread: client socket: %d\n", client_sock[counter]);
        counter++;

        printf("counter %d\n", counter);

        if(counter == 2)
            break;
    }

    if (client_sock[counter] < 0){
        perror("accept failed"); 
        close(socketFd);
        return ;   
    }

    close(socketFd);

    printf("thread exit\n");
    //printf("socket desc: %d\n", *(int*)socket_desc);
}

// return port number if exist, 0 if not
int check_operation(int op){

    /*
    printf("op_check: %d\n ", op_check[op]);
    printf("op: %d\n", op);
    */

    if( op_check[op] != 0 )
        return op_check[op];

    return 0;
}