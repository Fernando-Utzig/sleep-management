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


void ReceiveInterruption(int signalvalue)
{
    printf("Closing Program\n");
    fflush(stdout);
    keepRunning =0;
}
int main(int argc, char *argv[]){
    pthread_t discoveryThreadId =0, interfaceParticipantThreadId=0, interfaceManagerThreadId=0, monitoringThreadId=0, managementThreadId =0;
    init_participantTable();
    Participant *tmp;
    char read[64];
    printf("pthread_t value before = %ld\n",discoveryThreadId);
    signal(SIGINT,ReceiveInterruption);
    if (argc > 1 && strcmp(argv[1], "manager") == 0) {
        isManager = 1;
        printf("Estação iniciada como Manager\n");
        pthread_create(&discoveryThreadId, NULL, discoveryThread, NULL);
        pthread_create(&interfaceManagerThreadId, NULL, interfaceThreadManager, NULL);
        printf("pthread_t value after = %ld\n",discoveryThreadId);
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
        while(keepRunning)
        {
        }
        if(sendExitPacket(&ManagerSock) == -1)
        {
            printf("\nFailed to send exit packet");
            exit(1);
        }
        fprintf(stderr,"printa aqui");
    }
    fprintf(stderr,"Closing Threads\n");
    fprintf(stderr,"Closing Thread discoveryThreadId %ld\n",discoveryThreadId);
    if(discoveryThreadId != 0)
        pthread_cancel(discoveryThreadId);
    fprintf(stderr,"Closing Thread monitoringThreadId %ld\n",monitoringThreadId);
    if(monitoringThreadId != 0)
        pthread_cancel(monitoringThreadId);
    fprintf(stderr,"Closing Thread interfaceThreadId %ld\n",interfaceManagerThreadId);
    if(interfaceManagerThreadId != 0)
        pthread_cancel(interfaceManagerThreadId);
    fprintf(stderr,"Threads Closed!\n");
    closeDiscoverySocket();
    closeMonitoringSocket();
    fprintf(stderr,"Sockets Closed!\n");
    printf("Goodbye User!\n");
    fflush(stdout);
    fflush(stderr);
};