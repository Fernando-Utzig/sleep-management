#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include "participants.h"
#include "discovery.h"
#include "monitoring.h"
#include "interface.h"
#include <signal.h>

#define MONITORING_PORT 8889
#define PARTICIPANT_TABLE_SIZE 64

struct sockaddr_in ManagerSock;

// Variáveis globais
int isManager = 0;
int keepRunning =1;

FILE *openLogFile(char *name)
{
    FILE *file;

    if(name == NULL)
    {
        fprintf(stderr,"got a null filname\n");
        return NULL;
    }
    int tries=0;
    int maxtries =20;
    int last_char = strlen(name) -5;
    char filename[256]; 
    strcpy(filename,name); // need as the way i am passing the argument the string is read-only
    do
    {
        
        file = fopen(filename,"w");
        tries++;
        filename[last_char] = filename[last_char] +1;
    } while (file == NULL && tries<maxtries);
    if(tries >=maxtries)
    {
        fprintf(stderr,"FAILED to open any logfile\n");
        return NULL;
    }
    return file;

}

void ReceiveInterruption(int signalvalue)
{
    printf("Closing Program\n");
    fflush(stdout);
    keepRunning =0;
}
int main(int argc, char *argv[]){
    pthread_t discoveryThreadId =0, interfaceParticipantThreadId=0, interfaceThreadId=0, monitoringThreadId=0, managementThreadId =0,displayThreadId=0;
    
    
    Participant *tmp;
    char read[64];
    printf("Starting ... \n");
    fflush(stdout);
    setDiscoveryLogFile(openLogFile("discoveryLog_2.txt"));
    setInterfaceLogFile(openLogFile("interfaceLog_2.txt"));
    setMonitoringLogFile(openLogFile("monitoringLog_2.txt"));
    setParticipantsLogFile(openLogFile("participantLog_2.txt"));
    init_participantTable();
    setMySelf();
    signal(SIGINT,ReceiveInterruption);
    if (argc > 1 && strcmp(argv[1], "manager") == 0) {
        isManager = 1;
        printf("Estação iniciada como Manager\n");
        pthread_create(&discoveryThreadId, NULL, discoveryThread, NULL);
        pthread_create(&interfaceThreadId, NULL, interfaceThreadManager, NULL);
        pthread_create(&displayThreadId, NULL, displayParticipantsTable, NULL);
        while(keepRunning)
        {
            //what we could use this for?
        }
    } else {
        int is_awaken = 1;
        if(sendDiscoverypackaged(&ManagerSock) == -1)
        {
            printf("\nFailed to discover Manager, Closing");
            exit(1);
        }
        pthread_create(&monitoringThreadId, NULL, ParticipantMonitoringThread, &ManagerSock);
        pthread_create(&interfaceParticipantThreadId, NULL, interfaceThreadParticipant, NULL);
        pthread_create(&interfaceThreadId, NULL, printManagerThread, NULL);
        while(keepRunning)
        {
        }
        printf("Sending Exit request\n");
        fflush(stdout);
        tmp = getManagerCopy();
        if(tmp != NULL)
            if(sendExitRequest(tmp) == 1)
                printf("Exit successful!\n");
            else
                printf("Exit failed\n");
    }
    fprintf(stderr,"Closing Threads\n");
    fprintf(stderr,"Closing Thread discoveryThreadId %ld\n",discoveryThreadId);
    if(discoveryThreadId != 0)
        pthread_cancel(discoveryThreadId);
    fprintf(stderr,"Closing Thread monitoringThreadId %ld\n",monitoringThreadId);
    if(monitoringThreadId != 0)
        pthread_cancel(monitoringThreadId);
    fprintf(stderr,"Closing Thread interfaceThreadId %ld\n",interfaceThreadId);
    if(interfaceThreadId != 0)
        pthread_cancel(interfaceThreadId);
    fprintf(stderr,"Closing Thread displayThreadId %ld\n",interfaceThreadId);
    if(displayThreadId != 0)
        pthread_cancel(displayThreadId);
    fprintf(stderr,"Threads Closed!\n");
    closeDiscoverySocket();
    closeMonitoringSocket();
    fprintf(stderr,"Sockets Closed!\n");
    printf("Goodbye User!\n");
    fflush(stdout);
    fflush(stderr);
};