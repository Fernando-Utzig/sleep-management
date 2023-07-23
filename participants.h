#ifndef H_PARTICIPANTS
#define H_PARTICIPANTS
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
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
void setMyselfActive();
void setMyselfSleep();
#endif