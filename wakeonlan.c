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
#include "election.h"

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

void destroyManagerAssets(pthread_t *discoveryThreadId,pthread_t *discoveryManagerThreadId,pthread_t *interfaceThreadId,pthread_t *displayThreadId)
{
    fprintf(stderr,"Closing Thread discoveryThreadId %ld\n",*discoveryThreadId);
    if(discoveryThreadId != 0)
    {
        pthread_cancel(*discoveryThreadId);
        discoveryThreadId=0;
    }
        
    fprintf(stderr,"Closing Thread interfaceThreadId %ld\n",*interfaceThreadId);
    if(interfaceThreadId != 0)
    {
        pthread_cancel(*interfaceThreadId);
        interfaceThreadId=0;
    }
    fflush(stderr);
    fprintf(stderr,"Closing Thread displayThreadId %ld\n",*displayThreadId);
    if(displayThreadId != 0)
    {
        pthread_cancel(*displayThreadId);
        displayThreadId=0;
    }
    fprintf(stderr,"Closing Thread monitoring threads \n");
    fflush(stderr);
    destroyAllMonitoringInfo();
    fprintf(stderr,"resetinglist \n");
    fflush(stderr);
    resetListForParticipant();
    fprintf(stderr,"closing discovery socket \n");
    fflush(stderr);
    closeDiscoverySocket();

}




void runAsManager(pthread_t *discoveryThreadId,pthread_t *discoveryManagerThreadId,pthread_t *interfaceThreadId,pthread_t *displayThreadId)
{
    
    printf("Estação iniciada como Manager\n");
    setMyselfAsManager();
    Operation_result op = AddParticipantToTable(getMyselfCopy());//including manager into the list
    setMySelfId(op.id);
    pthread_create(discoveryThreadId, NULL, discoveryThread, NULL);
    pthread_create(interfaceThreadId, NULL, interfaceThreadManager, NULL);
    pthread_create(displayThreadId, NULL, displayParticipantsTable, NULL);
    pthread_create(discoveryManagerThreadId, NULL, discoveryManagerThread, NULL);
    createAllMonitoringInfo();
    while(keepRunning==1 && isManager==1)
    {
        //what we could use this for?
    }
    destroyManagerAssets(discoveryThreadId,discoveryManagerThreadId,interfaceThreadId,displayThreadId);
    usleep(10);
}

void destroyParticipantAssets(pthread_t *monitoringThreadId,pthread_t *interfaceParticipantThreadId,pthread_t *interfaceThreadId)
{
    fprintf(stderr,"Closing Thread monitoringThreadId %ld\n",*monitoringThreadId);
    if(monitoringThreadId != 0)
    {
        pthread_cancel(*monitoringThreadId);
        monitoringThreadId=0;
    }
        
    fprintf(stderr,"Closing Thread interfaceParticipantThreadId %ld\n",*interfaceParticipantThreadId);
    if(interfaceParticipantThreadId != 0)
    {
        pthread_cancel(*interfaceParticipantThreadId);
        interfaceParticipantThreadId=0;
    }
        
    fprintf(stderr,"Closing Thread interfaceThreadId %ld\n",*interfaceThreadId);
    if(interfaceThreadId != 0)
    {
        pthread_cancel(*interfaceThreadId);
        interfaceThreadId=0;
    }
    closeDiscoverySocket();
    closeMonitoringSocket();
}

void runAsParticipant(pthread_t *monitoringThreadId,pthread_t *interfaceParticipantThreadId,pthread_t *interfaceThreadId, pthread_t *electionThreadId)
{
    setDiscoveryLogFile(openLogFile("Logs/discoveryLog_1.txt"));
    setInterfaceLogFile(openLogFile("Logs/interfaceLog_1.txt"));
    setMonitoringLogFile(openLogFile("Logs/monitoringLog_1.txt"));
    setParticipantsLogFile(openLogFile("Logs/participantLog_1.txt"));
    setElectionLogFile(openLogFile("Logs/electionLog_2.txt"));
    pthread_create(monitoringThreadId, NULL, ParticipantMonitoringThread, &ManagerSock);
    pthread_create(interfaceParticipantThreadId, NULL, interfaceThreadParticipant, NULL);
    pthread_create(interfaceThreadId, NULL, displayParticipantsTable, NULL);
    pthread_create(electionThreadId, NULL, RecieveElectionMessageThread, NULL);
    while(keepRunning==1 && isManager==0)
    {
    }
    destroyParticipantAssets(monitoringThreadId,interfaceParticipantThreadId,interfaceThreadId);
}

void changeToManager(int signalvalue)
{
    printf("Changin to manager\n");
    isManager=1;
}

void changeToParticipant(int signalvalue)
{
    printf("Changin to Participant\n");
    isManager=0;
}

void ReceiveInterruption(int signalvalue)
{
    printf("Closing Program\n");
    fflush(stdout);
    keepRunning =0;
}
int main(int argc, char *argv[]){
    pthread_t discoveryThreadId =0,discoveryManagerThreadId =0, interfaceThreadId=0, interfaceParticipantThreadId=0, monitoringThreadId=0, managementThreadId =0,displayThreadId=0, electionThreadId=0;
    
    
    Participant *tmp;
    char read[64];
    printf("Starting ... \n");
    fflush(stdout);
    if (argc > 1 && strcmp(argv[1], "manager") == 0) {
        setDiscoveryLogFile(openLogFile("Logs/discoveryLog_manager2.txt"));
        setInterfaceLogFile(openLogFile("Logs/interfaceLog_manager2.txt"));
        setMonitoringLogFile(openLogFile("Logs/monitoringLog_manager2.txt"));
        setParticipantsLogFile(openLogFile("Logs/participantLog_manager2.txt"));
        setElectionLogFile(openLogFile("Logs/electionLog_manager2.txt"));
    }
    else
    {
        setDiscoveryLogFile(openLogFile("Logs/discoveryLog_1.txt"));
        setInterfaceLogFile(openLogFile("Logs/interfaceLog_1.txt"));
        setMonitoringLogFile(openLogFile("Logs/monitoringLog_1.txt"));
        setParticipantsLogFile(openLogFile("Logs/participantLog_1.txt"));
        setElectionLogFile(openLogFile("Logs/electionLog_1.txt"));
    }
    
    init_participantList();
    setMySelf();
    signal(SIGINT,ReceiveInterruption);
    signal(SIGUSR2,changeToParticipant);
    signal(SIGUSR1,changeToManager);
    
    if (argc > 1 && strcmp(argv[1], "manager") == 0) {
        isManager = 1;
    } else {
        int is_awaken = 1;
        if(sendDiscoverypackaged(&ManagerSock) == -1)
        {
            printf("Failed to discover Manager, running as Manager\n");
            isManager = 1;
        }        
        else
            isManager = 0;
    }
    while(keepRunning)
    {
        if(isManager==1 && keepRunning==1)
            runAsManager(&discoveryThreadId,&discoveryManagerThreadId,&interfaceThreadId,&displayThreadId);
        else if(isManager==0 && keepRunning==1)
            runAsParticipant(&monitoringThreadId,&interfaceParticipantThreadId,&interfaceThreadId,&electionThreadId);
    }
    if(isManager==0)
    {
        printf("Sending Exit request\n");
        fflush(stdout);
        tmp = getManagerCopy();
        if(tmp != NULL)
            if(sendExitRequest(tmp) == 1)
                printf("Exit successful!\n");
            else
                printf("Exit failed\n");
    }
    closeDiscoverySocket();
    closeMonitoringSocket();
    fprintf(stderr,"Sockets Closed!\n");
    printf("Goodbye User!\n");
    fflush(stdout);
    fflush(stderr);
};