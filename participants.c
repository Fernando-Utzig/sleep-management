#ifndef C_PARTICIPANTS
#define C_PARTICIPANTS

#include "participants.h"



unsigned long hash(char *mac){
    unsigned long sum =0;
    for (int j=0;mac[j]!='\0';j++)
    {
        sum += mac[j];
    }
    return sum % TABLE_SIZE;
}

void init_participantTable(void)
{
    int i;
    for(i=0;i<TABLE_SIZE;i++)
    {
        ParticipantsTable[i]=NULL;
    }
    fprintf(stderr,"participants table iniciated");
} 

int get_index(char* Message,int message_lenght)
{
    int i =0;
    for(i=0;Message[i]!=','&&i<message_lenght;i++)
    {
    }
    return i;
}
void copystring(char *dest,char *source,int lenght)
{
    int i;
    for(i=0;i<lenght;i++)
    {
        dest[i]=source[i];
    }
    dest[lenght]='\0';
}
Participant *Create_Participant(char* Message,int message_lenght)
{
    fprintf(stderr,"printa isso Create_Participant\n");
    Participant* new_participant = (Participant *)malloc(sizeof( Participant));
    char temp[Participant_Name_size];
    int i =0;
    int mac_index;
    int ip_index;
    int hostname_index;
    mac_index = get_index(Message,message_lenght);
    fprintf(stderr,"ip_index bate em %c\n",Message[mac_index]);
    fprintf(stderr,"i eh = %d a mensagem é = %s\n", mac_index,Message);
    copystring(new_participant->MAC,Message,mac_index);
    ip_index = get_index(&Message[mac_index],message_lenght-mac_index);
    copystring(new_participant->Hostname,Message,mac_index);
    new_participant->is_awaken=1;
    new_participant->next=NULL;
    return new_participant;
}

int insert_in_next(Participant *old,Participant *new)
{
    if(old == NULL || new == NULL)
    {
        fprintf(stderr,"insert_in_next FAILED");
        return -1;
    }
    if(strcmp(old->MAC,new->MAC) == 0)
    {
        old->is_awaken=1;
        free(new);
        fprintf(stderr,"Already known Participant");
        return 2;
    }
    else{
        if(old->next ==NULL)
        {
            old->next = new;
            return 1;
        }
        else
            return insert_in_next(old->next,new);
    }
}

int AddParticipantToTable(char* Message,int message_lenght)
{
    int return_value;
    if(Message == NULL)
    {
        fprintf(stderr,"AddParticipantToTable message is NULL");
        return -1;
    }
    int computed_hash;
    fprintf(stderr,"deu erro foi?\n");
    printf("kkkkk\n");
    Participant* new_participant = Create_Participant(Message,message_lenght);
    computed_hash=hash(new_participant->MAC);
    fprintf(stderr,"computed_hash = %d",computed_hash);
    fflush(stderr);
    pthread_mutex_lock(&participantsMutex);
    if(ParticipantsTable[computed_hash] != NULL){
        fprintf(stderr,"Found someone at the table");
        if(strcmp(ParticipantsTable[computed_hash]->MAC,new_participant->MAC) == 0)
        {
            fprintf(stderr,"Already known Participant");
            ParticipantsTable[computed_hash]->is_awaken=1;
            free(new_participant);
            return_value=2;
        }
        else{
            return_value=insert_in_next(ParticipantsTable[computed_hash],new_participant);
        }
    }
    else
    {
        fprintf(stderr,"Space is empty!");
        ParticipantsTable[computed_hash] = new_participant;
        return_value=1;
    }
    pthread_mutex_unlock(&participantsMutex);
    return return_value;
}

void UpdateParticipantStatus(char* hostname, int is_awaken)
{
    
}
void RemoveParticipantFromTable(char* hostname)
{
}

void printParticipant(Participant *participant)
{
    if(participant == NULL)
        return;
    if(participant->Hostname !=NULL)
        printf("Hostname: %s ,",participant->Hostname);
    if(participant->ip_address !=NULL)
        printf(" IP: %s,",participant->ip_address);
    if(participant->MAC !=NULL)
        printf(" MAC: %s,",participant->MAC);
    if(participant->is_awaken ==0)
        printf(" Status: Sleeping");
    else
        printf(" Status: Active");
    printf("\n");
    printParticipant(participant->next);
}

#endif