#ifndef C_MONITORING
#define C_MONITORING

#include "monitoring.h"
#define PORT_CLIENT_MON 34033
#define PORT_MANAGER_MON 34025
#define BUFFER_SIZE_MON 1024
#define CONFIRMATION_TRIES 3



int manager_socket =-1;
pthread_mutex_t max_portMutex;
int max_port=4026;
FILE *monitoring_logfile ;

struct struct_Monitoring_response
{
    int confirmed;
    int version;
} typedef MonitorResponse;

void setMonitoringLogFile(FILE *file)
{
    if(file != NULL)
        monitoring_logfile =file;
    else
    {
        monitoring_logfile=stderr;
        fprintf(monitoring_logfile,"discovery got a null file, using std err instead \n");
    }
    fprintf(monitoring_logfile,"monitoring_logfile set\n");
}

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
        fprintf(monitoring_logfile, "trying to remove \\n from NULL string");
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
    fprintf(monitoring_logfile,"Creating socket");
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
        printf("Failed to bind the MONITORING socket.\n");
        close(sockfd);
        raise(SIGINT);
    }
    fprintf(monitoring_logfile,"Socket id = %d\n",sockfd);
    struct timeval tv;
    
    fprintf(monitoring_logfile,"Monitoring Socket Created\n");
    return sockfd;
}

void *ParticipantMonitoringThread(void *arg) {
    fprintf(monitoring_logfile,"\nStarting ParticipantMonitoringThread\n");
    struct sockaddr_in *clientAddr = (struct sockaddr_in *) arg;
    int sockfd = createSocketMon(PORT_CLIENT_MON,clientAddr);
    fprintf(monitoring_logfile,"Port Created\n");
    MonitorResponse response;
    List_Participant *request=getParticipant_list();
    Participant *self = getMyselfCopy();
    int version_now=request->list_version;
    socklen_t len = sizeof(struct sockaddr_in);
    int send_ret,n;
    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    int time_left=3;
    int i;    
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
    while (1) {
        n = recvfrom(sockfd, request,  sizeof(List_Participant), 0, (struct sockaddr*)clientAddr, &len);
        fprintf(monitoring_logfile," n= %d\n",n);
        if (n > 0) {
            time_left=3;
            fprintf(monitoring_logfile,"received packaged \n");
            fprintf(monitoring_logfile,"versao agora =%d, versao recebida: %d\n",version_now,request->list_version);
            fflush(monitoring_logfile);    
            if(request->list_version>version_now)
            {
                version_now=request->list_version;
                display();
            }
            
            response.confirmed=1;
            response.version=version_now;        
            send_ret =sendto(sockfd, &response, sizeof(MonitorResponse), 0,(struct sockaddr *) clientAddr, sizeof(struct sockaddr));
            fprintf(monitoring_logfile,"send_ret = %d\n",send_ret);
        }
        else
        {
            tv.tv_usec = 0;
            time_left--;
            fprintf(monitoring_logfile,"TIMEOUT to Manager time left = %d\n",time_left);
            
            
            if(time_left<=0)
            {
                fprintf(monitoring_logfile,"LOST MANAGER, begin election!\n");
                CallElection(NULL);
            }
            fflush(monitoring_logfile);
        }
        fflush(monitoring_logfile);
    }
    return NULL;
}


void *monitorParticipant(void *arg)
{
    MonitoringInfo *monoration = (MonitoringInfo *) arg;
    List_Participant *request=getParticipant_list();
    MonitorResponse response;
    int send_ret,receive_ret;
    fprintf(monitoring_logfile,"created monitorParticipant\n");
    struct sockaddr_in participantAddress;
    getParticipantAddress(monoration->participant,PORT_CLIENT_MON,&participantAddress); //this will read only, so no risk
    struct sockaddr_in responseAddress;
    socklen_t len = sizeof(responseAddress);
    int threadPort = createSocketMon(max_port,&participantAddress);
    pthread_mutex_lock(&max_portMutex);    
    max_port++;
    pthread_mutex_unlock(&max_portMutex);
    struct timeval tv;
    tv.tv_sec = 3;
    tv.tv_usec = 0;    
    setsockopt(threadPort, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
    while(1)
    {
        fprintf(monitoring_logfile,"Sending List_Participant to participant: %s list version : %d\n",monoration->participant->Hostname, request->list_version);
        send_ret =sendto(threadPort, request, sizeof(List_Participant), 0,(struct sockaddr *) &participantAddress, sizeof(struct sockaddr));
        if (send_ret < 0)
        {
            fprintf(monitoring_logfile,"ERROR sendto List_Participant: %d \n",send_ret);
        }
            
        else
        {
            receive_ret = recvfrom(threadPort, &response, sizeof(MonitorResponse), 0, (struct sockaddr *) &responseAddress, &len);
            if (receive_ret < 0)
            {
                monoration->time_to_sleep--;
                fprintf(monitoring_logfile,"failed to receive List_Participant from %s time_to_sleep: %d\n",monoration->participant->Hostname,monoration->time_to_sleep);
                if(monoration->time_to_sleep<=0 && monoration->participant->is_awaken==1)
                {
                    fprintf(monitoring_logfile," %s is now declared Sleeping\n",monoration->participant->Hostname);
                    monoration->participant->is_awaken=0;
                    updateParticipant(monoration->participant);
                }
                usleep(100);
            }
            else
            {
                monoration->time_to_sleep=3;
                if(monoration->participant->is_awaken==0)
                {
                    fprintf(monitoring_logfile," %s is now declared Active\n",monoration->participant->Hostname);
                    monoration->participant->is_awaken=1;
                    updateParticipant(monoration->participant);
                }
            }
        }
        fflush(monitoring_logfile);
        tv.tv_usec = 0;
    }

}


#endif