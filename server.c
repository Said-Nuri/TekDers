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
#include <math.h>
#define MAX_LENGTH 512

void *operation_thread(void *thread_info);
int check_operation(int op);
double op1(double param1, double param2);
double op2(double param1, double param2);
double op3(double param1, double param2);
double op4(double param1, double param2);

int op_check[4] = {0, 0, 0, 0};
static char global_ip[MAX_LENGTH];
/*
    arg1 : IP
    arg2 : port
    arg3 : Number of thread per server
*/

struct thread_info_t{
    int portNo;
    int opno;
};

int main(int argc, char const *argv[])
{
    int i, c;
    const char* ipAddress = argv[1];
    int portNo = atoi(argv[2]);
    int numofthread = atoi(argv[3]);
    pthread_t op_threads[numofthread];
    struct thread_info_t thread_info[numofthread];
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

    printf("Main socket created\n\n\n");

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
    printf("bind done\n");

    //Listen
    listen(socketFd , 3);

    c = sizeof(struct sockaddr_in);

    while( (client_sock[counter] = accept(socketFd, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {   
        
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
            puts("same op accepted");

            memset(message, 0 , sizeof(message));
            sprintf(message, "%d", op_check[atoi(recieve)]);

            //Send some data
            if( send(client_sock[counter] , message , strlen(message) , 0) < 0){
                puts("Send failed");
                return 1;
            }
        }else{

            puts("New client accepted");
            op_check[atoi(recieve)] = ++portNo;

            thread_info[counter].portNo = portNo;
            thread_info[counter].opno = atoi(recieve);

            //Create thread
            if( pthread_create( &op_threads[counter] , NULL ,  operation_thread , (void*) &thread_info[counter]) < 0)
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

            /*
            counter++;

            if(counter == 2)
                break;
            */
        }
    }

    close(socketFd);

    while(1){

    }
    return 0;
}

void *operation_thread(void *thread_info){
    struct thread_info_t* info = (struct thread_info_t*)thread_info;
    int client_number = 2;
    int port = info->portNo;
    int opno = info->opno;
    double param1, param2, result = 0;
    int c;
    int cur_sock;
    struct sockaddr_in server, client;
    int socketFd;
    int counter = 0;
    int client_sock[client_number];
    char sending[MAX_LENGTH], receive[MAX_LENGTH];
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(global_ip);
    server.sin_port = htons( port );

    printf("thread started\n");
    printf("portNo: %d\n", info->portNo);
    printf("opno: %d\n", info->opno);

    if( (socketFd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
    {
        perror("socket failed:");
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
        cur_sock = client_sock[counter];
        counter++;


        memset(receive, 0, sizeof(receive));

        if( recv(cur_sock , receive , MAX_LENGTH , 0) < 0){
            perror("recv failed");
            close(cur_sock);
            return;
        }

        param1 = atof(receive);

        printf("param1: %lf\n", param1);
        
        memset(receive, 0, sizeof(receive));

        if( recv(cur_sock , receive , MAX_LENGTH , 0) < 0){
            perror("recv failed");
            close(client_sock[counter]);
            return;
        }

        param2 = atof(receive);
        
        printf("param2: %lf\n", param2);
        
        printf("opno: %d\n", opno);

        switch(opno){
            case 1:
                result = op1(param1, param2);
                break;
            case 2:
                result = op2(param1, param2);
                break;
            case 3:
                result = op3(param1, param2);
                break;
            case 4:
                result = op4(param1, param2);
                break;
                
        }

        memset(sending, 0, MAX_LENGTH);
        sprintf(sending, "%.4lf", result);

        printf("send res: %s\n", sending);

        if( send(cur_sock , sending , MAX_LENGTH , 0) < 0){
            perror("send failed");
            close(cur_sock);
            return;
        }

        printf("counter %d\n", counter);

        if(counter == 2)
            break;
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

double op1(double param1, double param2){
    printf("op1 started\n");
    return sqrt(param1 + param2);
}

double op2(double param1, double param2){
    printf("op2 started\n");

    return sin(param1/param2);
}

double op3(double param1, double param2){
    printf("op3 started\n");

    return fabs(param1-param2);
}

double op4(double param1, double param2){
    printf("op4 started\n");
    
    return 100;
}
