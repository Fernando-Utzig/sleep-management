#ifndef H_PARTICIPANTS
#define H_PARTICIPANTS
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#define Participant_Name_size 64
#define TABLE_SIZE 997
struct ParticipantData {
    char Hostname[Participant_Name_size];
    char MAC[Participant_Name_size];
    char ip_address[Participant_Name_size];
    int is_awaken;
    struct ParticipantData *next;
}typedef Participant;



int AddParticipantToTable(char* Message,int message_lenght);
unsigned long hash(char *mac);
void init_participantTable(void);
void printParticipant(Participant *participant);
void printAllParticipants();
Participant *getParticipant(char *Mac);
int updateParticipant(Participant *participant);
Participant *Create_Participant(char* Message,int message_lenght);
void printManager();
void setManager(char* Message,int message_lenght);
void setMyselfActive();
void setMyselfSleep();
void removeParticipantFromTable(char* hostname);
#endif