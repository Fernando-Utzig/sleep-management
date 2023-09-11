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
#define LIST_SIZE 64




struct monitoringInformation typedef MonitoringInfo;
struct ParticipantData {
    int id;
    char Hostname[Participant_Name_size];
    char MAC[Participant_Name_size];
    char ip_address[Participant_Name_size];
    int is_awaken;
    int is_manager;
    MonitoringInfo *monitoration;
}typedef Participant;

struct monitoringInformation
{
    Participant *participant;
    pthread_mutex_t participantMutex; //not used, will be for next fix
    int time_to_sleep;
    pthread_t monitoringThread;    
} typedef MonitoringInfo;

struct Participant_List_Metadata {
    Participant list[LIST_SIZE];
    int list_size;
    int list_version;
    int maxId;
} typedef List_Participant;

struct Operation_result_S
{
    int result;
    int id;
} typedef Operation_result;


void setParticipantsLogFile(FILE *file);
Operation_result AddParticipantToTable(Participant *participant);
Operation_result removeParticipantFromTable(Participant *participant);
unsigned long hash(char *mac);
void init_participantList(void);
void printParticipant(Participant *participant);
void printAllParticipants();
Participant *getParticipant(char *Mac);
Operation_result updateParticipant(Participant *participant);
Participant *Create_Participant(char* Message,int message_lenght);
void printManager();
void setMySelf();
void setMySelfIpOnLan(char *ip);
void setMySelfId(int id);
void setManager(Participant *received);
void setMyselfActive();
void setMyselfSleep();
void setMyselfAsManager();
Participant *getMyselfCopy();
Participant *getManagerCopy();
List_Participant* getParticipant_list();
void copyParticipant();
struct sockaddr_in *getParticipantAddress(Participant *participant,int port);

#endif