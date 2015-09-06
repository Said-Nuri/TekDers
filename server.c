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
#include <pthread.h>
#define MAX_LENGTH 512
#define DEBUG   

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
    int thread_res;
    int numofthread = atoi(argv[3]);
    int MAX_PER_THREAD = numofthread*2;
    int thread_counter = 0;
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

    #ifdef DEBUG
        printf("Main socket created\n\n");
    #endif
    

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

    #ifdef DEBUG
    printf("bind done\n");
    #endif

    //Listen
    listen(socketFd , 3);

    c = sizeof(struct sockaddr_in);

    while( (client_sock[counter] = accept(socketFd, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {   
        
        memset(recieve, 0 , sizeof(recieve));

        //Receive op no from client
        if( recv(client_sock[counter] , recieve , MAX_LENGTH , 0) < 0){
            perror("recv failed");
            break;
        }

        //printf("op no: %s\n", recieve);

        // check op code
        port_sent = check_operation(atoi(recieve));

        //printf("port_sent %d\n", port_sent);
        
        if(port_sent != 0){
            #ifdef DEBUG
            puts("same op accepted");
            #endif

            memset(message, 0 , sizeof(message));
            sprintf(message, "%d", op_check[atoi(recieve)]);

            //Send port number to client
            if( send(client_sock[counter] , message , strlen(message) , 0) < 0){
                puts("Send failed");
                return 1;
            }
            
        }else{
            #ifdef DEBUG
            puts("New client accepted");
            #endif

            //Refuse the client if want to do third op, sent -1
            if(counter == numofthread){
                memset(memset, 0, MAX_LENGTH);
                sprintf(message, "%d", -1);
                if( send(client_sock[counter] , message , strlen(message) , 0) < 0){
                    puts("Send failed");
                    return -1;
                }
                break;
            }

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

            
            counter++;
        }

        thread_counter++;
        
        #ifdef DEBUG
        printf("thread_counter %d\n", thread_counter);
        #endif

        if(thread_counter == MAX_PER_THREAD)
            break;
    }

    #ifdef DEBUG
    puts("Service ends");
    puts("Wait for threads die");
    #endif
    
    for (i = 0; i < numofthread; ++i)
    {
        pthread_join(op_threads[i], NULL); 
    }

    close(socketFd);

    return 0;
}

void *operation_thread(void *thread_info){
    struct thread_info_t* info = (struct thread_info_t*)thread_info;
    struct sockaddr_in server, client;
    double param1, param2, result = 0;
    unsigned int thread_id;
    int client_number = 2;
    int port = info->portNo;
    int opno = info->opno;
    int c;
    int cur_sock;
    int socketFd;
    int counter = 0;
    int client_sock[client_number];
    char sending[MAX_LENGTH], receive[MAX_LENGTH];
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(global_ip);
    server.sin_port = htons( port );

    #ifdef DEBUG
    printf("thread started\n");
    printf("portNo: %d\n", info->portNo);
    printf("opno: %d\n", info->opno);
    #endif

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

    #ifdef DEBUG
    printf("listening...\n");
    #endif

    while( (client_sock[counter] = accept(socketFd, (struct sockaddr *)&client, (socklen_t*)&c)) ){
        #ifdef DEBUG
        puts("Connection accepted");
        printf("thread: client socket: %d\n", client_sock[counter]);
        #endif       
        
        cur_sock = client_sock[counter];
        counter++;


        memset(receive, 0, sizeof(receive));

        if( recv(cur_sock , receive , MAX_LENGTH , 0) < 0){
            perror("recv failed");
            close(cur_sock);
            return;
        }

        param1 = atof(receive);
        
        #ifdef DEBUG
        printf("param1: %lf\n", param1);
        #endif
        
        memset(receive, 0, sizeof(receive));

        if( recv(cur_sock , receive , MAX_LENGTH , 0) < 0){
            perror("recv failed");
            close(client_sock[counter]);
            return;
        }

        param2 = atof(receive);
        
        #ifdef DEBUG
        printf("param2: %lf\n", param2);
        printf("opno: %d\n", opno);
        #endif

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

        /********** Send Result ************/
        memset(sending, 0, MAX_LENGTH);
        sprintf(sending, "%.4lf", result);

        #ifdef DEBUG
        printf("send result: %s\n", sending);
        #endif

        if( send(cur_sock , sending , MAX_LENGTH , 0) < 0){
            perror("send failed");
            close(cur_sock);
            return;
        }
        /**************************************/
       
        /********** Send thread ID ************/
        thread_id = (unsigned int)pthread_self();
        memset(sending, 0, MAX_LENGTH);
        sprintf(sending, "%d", thread_id);

        #ifdef DEBUG
        printf("thread id: %s\n", sending);
        #endif

        if( send(cur_sock , sending , MAX_LENGTH , 0) < 0){
            perror("send failed");
            close(cur_sock);
            return;
        }
        /**************************************/

        #ifdef DEBUG
        printf("counter %d\n", counter);
        #endif

        if(counter == 2)
            break;
    }

    close(socketFd);

    #ifdef DEBUG
    printf("thread exit\n");
    #endif

    pthread_exit(NULL);
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
    #ifdef DEBUG
    printf("op1 started\n");
    #endif
    return sqrt(param1 + param2);
}

double op2(double param1, double param2){
    #ifdef DEBUG
    printf("op2 started\n");
    #endif

    return sin(param1/param2);
}

double op3(double param1, double param2){
    #ifdef DEBUG
    printf("op3 started\n");
    #endif

    return fabs(param1-param2);
}

double op4(double param1, double param2){
    #ifdef DEBUG
    printf("op4 started\n");
    #endif

    return (double)cos(param2)*(-10);
}
