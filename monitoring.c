
#ifndef C_MONITORING
#define C_MONITORING
#include "monitoring.h"
#define PORT_CLIENT_MON 4024
#define BUFFER_SIZE_MON 1024

int compare_all(char* u,char*d,int len)
{
    int i;
    for(i=0;i<len;i++)
    {
        printf("i=%d u=%c d=%c \n",i,u[i],d[i]);
    }
}


int createSocketMon(int port, struct sockaddr_in *Manageraddress) {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    struct hostent *server;
    if (sockfd < 0) {
        printf("Failed to create socket.");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    //serverAddr.sin_addr = *((struct in_addr *) &Manageraddress->sin_addr);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    
    bzero(&(serverAddr.sin_zero), 8); 
    if (bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        printf("Failed to bind the socket.");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    fprintf(stderr,"Socket id = %d",sockfd);
    fprintf(stderr,"\tDiscovery Socket Created\n");
    return sockfd;
}

void *ParticipantMonitoringThread(void *arg) {
    fprintf(stderr,"\nStarting ParticipantMonitoringThread\n");
    struct sockaddr_in *clientAddr = (struct sockaddr_in *) arg;
    int sockfd = createSocketMon(PORT_CLIENT_MON,clientAddr);
    fprintf(stderr,"Port Created\n");
    
    char buffer[BUFFER_SIZE_MON];
    socklen_t len = sizeof(struct sockaddr_in);
    int send_ret,n;
    char *response;
    while (1) {
        memset(buffer, 0, BUFFER_SIZE_MON);
        response="Invalid Command received\n";
        n = recvfrom(sockfd, buffer, BUFFER_SIZE_MON, 0, (struct sockaddr*)clientAddr, &len);
        fprintf(stderr," n= %d",n);
        if (n > 0) {
            fprintf(stderr,"received packaged ");
            buffer[n-1] = '\0';
            fprintf(stderr,"message = %s\n",buffer);
            if(strcmp(buffer,"sleep")==0)
                response="Sleep Command received\n";
            if(strcmp(buffer,"wakeup")==0)
                response="Wakeup Command received\n";
            compare_all(buffer,"wakeup",n);
            fprintf(stderr,"response= %d\n",strcmp(buffer,"wakeup"));
            send_ret =sendto(sockfd, response, strlen(response), 0,(struct sockaddr *) clientAddr, sizeof(struct sockaddr));
        
            fprintf(stderr,"send_ret = %d",send_ret);
        }
        fflush(stderr);
    }
}





#endif