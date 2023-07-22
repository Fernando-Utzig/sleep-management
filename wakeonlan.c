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
    pthread_t discoveryThreadId, interfaceThreadId, monitoringThreadId, managementThreadId;
    init_participantTable();
    Participant *tmp;
    char read[64];
    signal(SIGINT,ReceiveInterruption);
    if (argc > 1 && strcmp(argv[1], "manager") == 0) {
        isManager = 1;
        printf("Estação iniciada como Manager\n");
        pthread_create(&discoveryThreadId, NULL, discoveryThread, NULL);
        pthread_create(&interfaceThreadId, NULL, interfaceThreadManager, NULL);
        while(keepRunning)
        {
            
        }
        pthread_cancel(discoveryThreadId);
        pthread_cancel(interfaceThreadId);
        fprintf(stderr,"Threads Closed\n");
        return 0;
    } else {
        int is_awaken = 1;
        if(sendDiscoverypackaged(&ManagerSock) == -1)
        {
            printf("\nFailed to discover Manager, Closing");
            exit(1);
        }
        pthread_create(&monitoringThreadId, NULL, ParticipantMonitoringThread, &ManagerSock);
        pthread_create(&interfaceThreadId, NULL, interfaceThreadParticipant, NULL);
        while(keepRunning)
        {

        }
    }
};