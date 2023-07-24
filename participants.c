#ifndef C_PARTICIPANTS
#define C_PARTICIPANTS
#define TABLE_SIZE 997

#include "participants.h"

Participant* ParticipantsTable[TABLE_SIZE];
pthread_mutex_t participantsMutex;
Participant myself;
pthread_mutex_t myselfMutex;




void printAllParticipants()
{
    pthread_mutex_lock(&participantsMutex);
    for (int i=0; i<TABLE_SIZE; i++) {
        printParticipant(ParticipantsTable[i]);
    }
    pthread_mutex_unlock(&participantsMutex);
}
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
    fprintf(stderr,"participants table iniciated\n");
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
    fprintf(stderr,"i eh = %d a mensagem é = %s\n", mac_index,Message);
    fprintf(stderr,"mac_index=%d bate em %c\n",mac_index,Message[mac_index]);
    copystring(new_participant->MAC,Message,mac_index);
    hostname_index = get_index(&Message[mac_index+1],message_lenght-mac_index)+mac_index+1;
    fprintf(stderr,"ip_index=%d bate em %c\n",hostname_index,Message[hostname_index]);
    copystring(new_participant->Hostname,&Message[mac_index+1],hostname_index - mac_index - 1);
    ip_index = get_index(&Message[hostname_index+1],message_lenght-hostname_index);
    copystring(new_participant->ip_address,&Message[hostname_index+1],ip_index);
    new_participant->is_awaken=1;
    new_participant->next=NULL;
    printParticipant(new_participant);
    return new_participant;
}

//beware to lock the table before using this
Participant *find_in_next(Participant *root,char *Mac)
{
    if(root == NULL)
        return NULL;
    if(strcmp(root->MAC,Mac))
    {
        return root;
    }
    else
        return find_in_next(root->next,Mac);
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
    Participant* new_participant = Create_Participant(Message,message_lenght);
    computed_hash=hash(new_participant->MAC);
    fprintf(stderr,"computed_hash = %d\n",computed_hash);
    fflush(stderr);
    pthread_mutex_lock(&participantsMutex);
    if(ParticipantsTable[computed_hash] != NULL){
        fprintf(stderr,"Found someone at the table\n");
        if(strcmp(ParticipantsTable[computed_hash]->MAC,new_participant->MAC) == 0)
        {
            fprintf(stderr,"Already known Participant\n");
            ParticipantsTable[computed_hash]->is_awaken=1;
            fprintf(stderr,"seted particitipant as awake\n");
            fflush(stderr);
            free(new_participant);
            fprintf(stderr,"free new_participant\n");
            return_value=2;
        }
        else{
            return_value=insert_in_next(ParticipantsTable[computed_hash],new_participant);
        }
    }
    else
    {
        fprintf(stderr,"Space is empty!\n");
        ParticipantsTable[computed_hash] = new_participant;
        return_value=1;
    }
    pthread_mutex_unlock(&participantsMutex);
    return return_value;
}



int updateParticipant(Participant *participant)
{
    Participant *tmp;
    int result;
    if(participant == NULL)
    {
        fprintf(stderr,"Trying to update NULL participant\n");
        return -1;
    }
    int computed_hash = hash(participant->MAC);
    pthread_mutex_lock(&participantsMutex);
    if(ParticipantsTable[computed_hash]==NULL)
        result = -1;
    else
    {
        if(strcmp(ParticipantsTable[computed_hash]->MAC,participant->MAC) == 0)
            ParticipantsTable[computed_hash]->is_awaken=participant->is_awaken;
        else{
            tmp = find_in_next(ParticipantsTable[computed_hash]->next,participant->MAC);
            if(tmp==NULL)
                result = -2;
            else
            {
                tmp->is_awaken=participant->is_awaken;
                result=1;
            }
        }    
    }    
    pthread_mutex_unlock(&participantsMutex);
    return result;

}
void removeParticipantFromTable(char* hostname)
{
}

void printParticipant(Participant *participant)
{
    if (participant == NULL)
        return;
    int headerPrinted = 0;
    if (!headerPrinted) {
        printf("-------------------------\n");
        printf("      Participants       \n");
        printf("--------------------------\n");
        headerPrinted = 1; 
    }

    if (participant->Hostname != NULL)
        printf("Hostname: %s\n", participant->Hostname);
        printf("-------------------------\n");
    if (participant->ip_address != NULL)
        printf("IP: %s\n", participant->ip_address);
        printf("-------------------------\n");
    if (participant->MAC != NULL)
        printf("MAC: %s\n", participant->MAC);
        printf("-------------------------\n");

    printf("Status: ");
    if (participant->is_awaken == 0)
        printf("Sleeping\n");
    else
        printf("Active\n");
    printf("-------------------------\n");
    printParticipant(participant->next);
}

Participant * deepCopyParticipant(Participant *original)
{
    Participant *tmp = (Participant *) malloc(sizeof(Participant));
    strcpy(tmp->Hostname,original->Hostname);
    strcpy(tmp->ip_address,original->ip_address);
    strcpy(tmp->MAC,original->MAC);
    tmp->is_awaken=original->is_awaken;
    return tmp;
}



Participant *getParticipant(char *Mac)
{
    Participant *tmp;
    if(Mac == NULL)
    {
        fprintf(stderr,"ERROR Mac is null ");
        return NULL;
    }
    int computed_hash=hash(Mac);
    pthread_mutex_lock(&participantsMutex);
    if(ParticipantsTable[computed_hash]==NULL)
        return NULL;
    if(strcmp(ParticipantsTable[computed_hash]->MAC,Mac) == 0)
        tmp=deepCopyParticipant(ParticipantsTable[computed_hash]);
    else
        tmp= deepCopyParticipant(find_in_next(ParticipantsTable[computed_hash]->next,Mac));        
    pthread_mutex_unlock(&participantsMutex);
    return tmp;
}

void setMyselfSleep()
{
    pthread_mutex_lock(&myselfMutex);
    myself.is_awaken=0;
    pthread_mutex_unlock(&myselfMutex);
}

void setMyselfActive()
{
    pthread_mutex_lock(&myselfMutex);
    myself.is_awaken=1;
    pthread_mutex_unlock(&myselfMutex);
}


#endif