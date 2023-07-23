#ifndef C_MONITORING
#define C_MONITORING

#include "monitoring.h"
#define PORT_CLIENT_MON 4024
#define PORT_MANAGER_MON 4025
#define BUFFER_SIZE_MON 1024
#define CONFIRMATION_TRIES 3

int manager_socket =-1;
int compare_all(char* u,char*d,int len)
{
    int i;
    for(i=0;i<len;i++)
    {
        printf("i=%d u=%c d=%c \n",i,u[i],d[i]);
    }
}

void closeMonitoringSocket()
{
    if(manager_socket != -1)
        close(manager_socket);
    return;
}
void removeEnterCharMon(char *string)
{
    if(string == NULL)
    {
        fprintf(stderr, "trying to remove \\n from NULL string");
        return;
    }
    int i;
    for(i=0;string[i]!='\n'&& string[i]!='\0';i++)
    {}
    if(string[i]=='\n')
        string[i]='\0';
    return;
}

int createSocketMon(int port, struct sockaddr_in *Manageraddress) {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    struct hostent *server;
    if (sockfd < 0) {
        printf("Failed to create socket.\n");
        raise(SIGINT);
    }
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    //serverAddr.sin_addr = *((struct in_addr *) &Manageraddress->sin_addr); //makes accept only the manager
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    
    bzero(&(serverAddr.sin_zero), 8); 
    if (bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        printf("Failed to bind the socket.\n");
        close(sockfd);
        raise(SIGINT);
    }
    fprintf(stderr,"Socket id = %d\n",sockfd);
    struct timeval tv;
    
    fprintf(stderr,"Monitoring Socket Created\n");
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
        fprintf(stderr," n= %d\n",n);
        if (n > 0) {
            fprintf(stderr,"received packaged ");
            removeEnterCharMon(buffer);
            fprintf(stderr,"message = %s\n",buffer);
            if(strcmp(buffer,"sleep")==0)
            {
                response="Sleep Command received\n";
                setMyselfSleep();
            }
                
            if(strcmp(buffer,"wakeup")==0)
            {
                response="Wakeup Command received\n";
                setMyselfActive();
            }
                
            fprintf(stderr,"response= %d\n",strcmp(buffer,"wakeup"));
            send_ret =sendto(sockfd, response, strlen(response), 0,(struct sockaddr *) clientAddr, sizeof(struct sockaddr));
            fprintf(stderr,"send_ret = %d\n",send_ret);
        }
        fflush(stderr);
    }
    return NULL;
}

struct sockaddr_in *getParticipantAddress(Participant *participant)
{
    if(participant == NULL)
    {
        fprintf(stderr,"Error on getting participant address, participant is null \n");
        return NULL;
    }
    if(participant->ip_address == NULL )
    {
        fprintf(stderr,"Error on getting participant address, participant ip_address is null \n");
        return NULL;
    }
    struct sockaddr_in *serverAddr = (struct sockaddr_in *) malloc(sizeof(struct sockaddr_in));
    int worked = inet_aton("127.0.1.1",&serverAddr->sin_addr);
    if(worked != 0 && strcmp(participant->ip_address,"127.0.1.1"))
    {
        fprintf(stderr,"Error on getting participant address, participant ip_address is Invalid. Address: %s \n",participant->ip_address);
        return NULL;
    }
    serverAddr->sin_family = AF_INET;
    serverAddr->sin_port = htons(PORT_CLIENT_MON);
    return serverAddr;
}



int sendRequest(char* request,Participant *participant)
{
    fprintf(stderr,"in SendRequest\n");
    int send_ret,receive_ret, request_result;
    char buffer[BUFFER_SIZE_MON];
    
    int tries =0;
    struct sockaddr_in *participantAddress = getParticipantAddress(participant);
    struct sockaddr_in Manageraddress;
    
    if(participantAddress == NULL)
    {
        fprintf(stderr,"Failed to send request: participantAddress is NULL\n");
        return -1;
    }
    socklen_t len = sizeof(Manageraddress);
    if(request == NULL)
    {
        fprintf(stderr,"Failed to send request: request is NULL\n");
        return -1;
    }
    if( manager_socket == -1)
        manager_socket = createSocketMon(PORT_MANAGER_MON,participantAddress);
    struct timeval tv;
    tv.tv_sec = 3;
    tv.tv_usec = 0;    
    setsockopt(manager_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);    
    do
    {
        fprintf(stderr,"Sending Request try(%d): %s\n",tries,request);
        send_ret =sendto(manager_socket, request, strlen(request), 0,(struct sockaddr *) participantAddress, sizeof(struct sockaddr));
        if (send_ret < 0) 
            fprintf(stderr,"ERROR sendto: %d \n",send_ret);
        else
        {
            receive_ret = recvfrom(manager_socket, buffer, BUFFER_SIZE_MON, 0, (struct sockaddr *) &Manageraddress, &len);
        }
        
        tv.tv_usec = 0;
        tries++;
    } while((send_ret <0 || receive_ret <0) && tries <CONFIRMATION_TRIES);
    if(tries >=CONFIRMATION_TRIES)
        request_result = -1;
    else
        request_result =1;
    
    free(participantAddress);
    return request_result;
}


int sleepOrWakupParticipant(Participant *participant, int new_status)
{
    int update_result;
    int request_result;
    if(new_status ==1)
        request_result = sendRequest("sleep",participant);
    if(new_status == 0)
        request_result =  sendRequest("wakeup",participant);
    if(request_result == -1)
        return -1;
    else
    {
        if(request_result == 1)
        {
            participant->is_awaken=new_status;
            update_result = updateParticipant(participant);
        }
    }
    if(update_result =1 && request_result ==1)
        return 1;
    else
        return -1;
}


#endif