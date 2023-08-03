#ifndef H_PARTICIPANTS
#define H_PARTICIPANTS
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include "interface.h"
#include "monitoring.h"

#define Participant_Name_size 64
#define TABLE_SIZE 997


struct monitoringInformation typedef MonitoringInfo;

struct ParticipantData {
    char Hostname[Participant_Name_size];
    char MAC[Participant_Name_size];
    char ip_address[Participant_Name_size];
    int is_awaken;
    struct ParticipantData *next;
    struct MonitoringInfo *monitoration;
}typedef Participant;

struct monitoringInformation
{
    Participant *participant;
    pthread_mutex_t participantMutex; //not used, will be for next fix
    int time_to_sleep;
    pthread_t monitoringThread;    
} typedef MonitoringInfo;

void setParticipantsLogFile(FILE *file);
int AddParticipantToTable(Participant *participant);
int removeParticipantFromTable(Participant *participant);
unsigned long hash(char *mac);
void init_participantTable(void);
void printParticipant(Participant *participant);
void printAllParticipants();
Participant *getParticipant(char *Mac);
int updateParticipant(Participant *participant);
Participant *Create_Participant(char* Message,int message_lenght);
void printManager();
void setMySelf();
void setMySelfIpOnLan(char *ip);
void setManager(Participant *received);
void setMyselfActive();
void setMyselfSleep();
Participant *getMyselfCopy();
Participant *getManagerCopy();
void copyParticipant();
struct sockaddr_in *getParticipantAddress(Participant *participant,int port);

#endif