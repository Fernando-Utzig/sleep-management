#ifndef C_PARTICIPANTS
#define C_PARTICIPANTS
#define TABLE_SIZE 997

#include "participants.h"

Participant* ParticipantsTable[TABLE_SIZE];
pthread_mutex_t participantsMutex;
Participant myself;
Participant *Manager;
pthread_mutex_t myselfMutex;


Participant * CreateCopyParticipant(Participant *original);
void createMonitoringInfo(Participant *participant);

FILE *participant_logfile ;
void setParticipantsLogFile(FILE *file)
{
    if(file != NULL)
        participant_logfile =file;
    else
    {
        participant_logfile=stderr;
        fprintf(participant_logfile,"discovery got a null file, using std err instead \n");
    }
    fprintf(participant_logfile,"participant_logfile set\n");
}

void printAllParticipants()
{
    
    pthread_mutex_lock(&participantsMutex);
    printf("\n");
    printf("-------------------------\n");
    printf("      Participants       \n");
    printf("-------------------------\n");
    printf("Hostname  |  IP  |  MAC  |  Status\n");
    for (int i=0; i<TABLE_SIZE; i++) {
        printParticipant(ParticipantsTable[i]);
    }
    pthread_mutex_unlock(&participantsMutex);
}

void setManager(Participant *received)
{
    Manager = CreateCopyParticipant(received);
    Manager->next=NULL;
}



void getMyMac(char *myMac)
{
    if(myMac == NULL)
    {
        printf("Trying to get mac with NULL string\n");
        return;
    }
        
    DIR *dp;
    struct dirent *ep;
    char address_path[256] = "/sys/class/net/";
    dp = opendir ("/sys/class/net/");
    if (dp == NULL)
    {
        printf("MY MAC READ FAILED- failed to read directory /sys/class/net/\n");
        strcpy(myMac,"AA:BB:CC:DD:EE:FF");
        return;
    }
    do
    {
        ep = readdir (dp);
    } while (ep != NULL && (ep->d_name[0]=='.' || !(strcmp(ep->d_name,"lo"))));
    if(ep == NULL)
    {
        printf("MY MAC READ FAILED- failed to get connection folder in /sys/class/net/\n");
        strcpy(myMac,"AA:BB:CC:DD:EE:FF");
        return;
    }
    
    fprintf(participant_logfile,"ep = %s\n",ep->d_name);
    strcat(address_path,ep->d_name);
    strcat(address_path,"/address");
    fprintf(participant_logfile,"address path used = %s\n",address_path);
    FILE * eth0 = fopen(address_path, "r");
	if(eth0 == NULL)
	{
		printf("MY MAC READ FAILED- to open file\n");
        strcpy(myMac,"AA:BB:CC:DD:EE:FF");
        return;
	}

    if(fgets(myMac, 18, eth0) == NULL )// getting only the mac, which is the 18 first char
    {
        printf("MY MAC READ FAILED-to read file\n");
        strcpy(myMac,"AA:BB:CC:DD:EE:FF");
    }
    printf("mymac = %s\n",myMac);
    return;
}

void setMySelfIpOnLan(char *ip)
{
    if(ip !=NULL)
    {
        strcpy(myself.ip_address,ip);
    }
}

void setMySelf()
{
    struct hostent *myhost;
    fprintf(participant_logfile,"Getting My Participant Info\n");
    char *myip;
    getMyMac(myself.MAC);
    if(gethostname(myself.Hostname,sizeof(myself.Hostname)) == -1)
    {
        printf("MY HOSTNAME READ FAILED\n");
        strcpy(myself.Hostname,"MyDefaultaHostname");
        strcpy(myself.ip_address,"10.1.1.1");
    }
    else
    {
        myhost = gethostbyname(myself.Hostname);
        myip = inet_ntoa(*((struct in_addr*)
                        myhost->h_addr_list[0]));
        strcpy(myself.ip_address,myip);
        myself.is_awaken = 1;

    }
	
    
    fprintf(participant_logfile,"MyMac is =%s\n",myself.MAC);
    fprintf(participant_logfile,"hostname is = %s\n",myself.Hostname);
    fprintf(participant_logfile,"myip is = %s\n",myself.ip_address);
    myself.next = NULL;
}
void printManager()
{
    system("clear"); // Comando para limpar a tela no Ubuntu
    printf("\n");
    printf("-------------------------\n");
    printf("        Manager          \n");
    printf("-------------------------\n");
    printf("Hostname  |  IP  |  MAC  |  Status\n");
    printParticipant(Manager);
}

unsigned long hash(char *hostname){
    unsigned long sum =0;
    for (int j=0;hostname[j]!='\0';j++)
    {
        sum += hostname[j];
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
    fprintf(participant_logfile,"participants table iniciated\n");
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
    fprintf(participant_logfile,"printa isso Create_Participant\n");
    Participant* new_participant = (Participant *)malloc(sizeof( Participant));
    char temp[Participant_Name_size];
    int i =0;
    int mac_index;
    int ip_index;
    int hostname_index;
    mac_index = get_index(Message,message_lenght);
    fprintf(participant_logfile,"i eh = %d a mensagem é = %s\n", mac_index,Message);
    fprintf(participant_logfile,"mac_index=%d bate em %c\n",mac_index,Message[mac_index]);
    copystring(new_participant->MAC,Message,mac_index);
    hostname_index = get_index(&Message[mac_index+1],message_lenght-mac_index)+mac_index+1;
    fprintf(participant_logfile,"ip_index=%d bate em %c\n",hostname_index,Message[hostname_index]);
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
    if(strcmp(root->Hostname,Mac))
    {
        return root;
    }
    else
        return find_in_next(root->next,Mac);
}

Participant *find_before_next(Participant *root,char *hostname) // untested
{
    if(root == NULL)
        return NULL;
    if(root->next == NULL)
        return NULL;
    if(strcmp(root->next->Hostname,hostname))
    {
        return root;
    }
    else
        return find_in_next(root->next,hostname);
}

int insert_in_next(Participant *old,Participant *new)
{
    if(old == NULL || new == NULL)
    {
        fprintf(participant_logfile,"insert_in_next FAILED");
        return -1;
    }
    if(strcmp(old->Hostname,new->Hostname) == 0)
    {
        old->is_awaken=1;
        free(new);
        fprintf(participant_logfile,"Already known Participant");
        return 2;
    }
    else{
        if(old->next ==NULL)
        {
            old->next = new;
            createMonitoringInfo(new);
            return 1;
        }
        else
            return insert_in_next(old->next,new);
    }
}

int AddParticipantToTable(Participant *participant)
{
    fprintf(participant_logfile,"Adding participant\n");
    int return_value;
    if(participant == NULL)
    {
        fprintf(participant_logfile,"AddParticipantToTable participant is NULL\n");
        return -1;
    }
    int computed_hash;
    Participant* new_participant = CreateCopyParticipant(participant);
    computed_hash=hash(new_participant->Hostname);
    fprintf(participant_logfile,"computed_hash = %d\n",computed_hash);
    fflush(participant_logfile);
    pthread_mutex_lock(&participantsMutex);
    if(ParticipantsTable[computed_hash] != NULL){
        fprintf(participant_logfile,"Found someone at the table\n");
        if(strcmp(ParticipantsTable[computed_hash]->Hostname,new_participant->Hostname) == 0)
        {
            fprintf(participant_logfile,"Already known Participant\n");
            ParticipantsTable[computed_hash]->is_awaken=1;
            fprintf(participant_logfile,"seted particitipant as awake\n");
            fprintf(participant_logfile,"free new_participant\n");
            return_value=2;
        }
        else{
            return_value=insert_in_next(ParticipantsTable[computed_hash],new_participant);
            
        }
    }
    else
    {
        fprintf(participant_logfile,"Space is empty!\n");
        ParticipantsTable[computed_hash] = new_participant;
        return_value=1;
        createMonitoringInfo(new_participant);
    }
    pthread_mutex_unlock(&participantsMutex);
    fprintf(participant_logfile,"Adding participant return value : %d\n",return_value);
    display();
    return return_value;
}



int updateParticipant(Participant *participant)
{
    Participant *tmp;
    int result;
    if(participant == NULL)
    {
        fprintf(participant_logfile,"Trying to update NULL participant\n");
        return -1;
    }
    int computed_hash = hash(participant->Hostname);
    pthread_mutex_lock(&participantsMutex);
    if(ParticipantsTable[computed_hash]==NULL)
        result = -1;
    else
    {
        if(strcmp(ParticipantsTable[computed_hash]->Hostname,participant->Hostname) == 0)
        {
            ParticipantsTable[computed_hash]->is_awaken=participant->is_awaken;
            result =1;
        }
            
        else{
            tmp = find_in_next(ParticipantsTable[computed_hash]->next,participant->Hostname);
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
    display();
    return result;

}
int removeParticipantFromTable(Participant *participant)
{
    
    Participant *tmp,*tmp2;
    fprintf(participant_logfile,"Removin Participant\n");
    int result;
    if(participant == NULL)
    {
        fprintf(participant_logfile,"Trying to remove NULL participant\n");
        return -1;
    }
    int computed_hash = hash(participant->Hostname);
    fprintf(participant_logfile,"computed hash = %d\n",computed_hash);
    fflush(participant_logfile);
    pthread_mutex_lock(&participantsMutex);
    if(ParticipantsTable[computed_hash]==NULL)
        result = -1;
    else
    {
        
        if(strcmp(ParticipantsTable[computed_hash]->Hostname,participant->Hostname) == 0)
            {
                result =1;
                if(ParticipantsTable[computed_hash]->next == NULL)
                {
                    fprintf(participant_logfile,"removin found in list, no next\n");
                    fflush(participant_logfile);
                    destroyMonitoringInfo(ParticipantsTable[computed_hash]);
                    free(ParticipantsTable[computed_hash]);
                    ParticipantsTable[computed_hash]=NULL;
                }
                else
                {
                    fprintf(participant_logfile,"removin found in list, next now in the list\n");
                    tmp = ParticipantsTable[computed_hash];
                    ParticipantsTable[computed_hash] = ParticipantsTable[computed_hash]->next;
                    destroyMonitoringInfo(ParticipantsTable[computed_hash]);
                    free(tmp);
                }
            }
        else{
            tmp = find_before_next(ParticipantsTable[computed_hash]->next,participant->Hostname);
            if(tmp==NULL)
                result = -2;
            else
            {//untested
                result =1;
                
                tmp2 = tmp->next;
                destroyMonitoringInfo(tmp2);
                tmp->next = tmp2->next;
                
                free(tmp2);
            }
        }    
    }
    pthread_mutex_unlock(&participantsMutex);
    display();
    fprintf(participant_logfile,"removing result: %d\n",result);
    return result;
}

void printParticipant(Participant *participant)
{
    if (participant == NULL)
        return;

    if (participant->Hostname != NULL)
        printf("%s | ", participant->Hostname);
    if (participant->ip_address != NULL)
        printf("%s | ", participant->ip_address);
    if (participant->MAC != NULL)
        printf("%s | ", participant->MAC);
    if (participant->is_awaken == 0)
        printf("Sleeping\n");
    else
        printf("Active\n");
    printf("-------------------------\n");
    printParticipant(participant->next);
}

void copyParticipant(Participant *copy,Participant *original)
{
    strcpy(copy->Hostname,original->Hostname);
    strcpy(copy->ip_address,original->ip_address);
    strcpy(copy->MAC,original->MAC);
    copy->is_awaken=original->is_awaken;
}

Participant * CreateCopyParticipant(Participant *original)
{
    Participant *tmp = (Participant *) malloc(sizeof(Participant));
    copyParticipant(tmp,original);
    return tmp;
}

Participant *getMyselfCopy()
{
    fprintf(participant_logfile,"Creatring copy of myself\n");
    Participant *copy;
    pthread_mutex_lock(&myselfMutex);
    copy = CreateCopyParticipant(&myself);
    pthread_mutex_unlock(&myselfMutex);
    return copy;
}

Participant *getManagerCopy()
{
    fprintf(participant_logfile,"Creatring copy of myself\n");
    Participant *copy;
    copy = CreateCopyParticipant(Manager);
    return copy;
}

Participant *getParticipant(char *hostname)
{
    Participant *tmp;
    if(hostname == NULL)
    {
        fprintf(participant_logfile,"ERROR Mac is null ");
        return NULL;
    }
    int computed_hash=hash(hostname);
    pthread_mutex_lock(&participantsMutex);
    if(ParticipantsTable[computed_hash]==NULL)
        return NULL;
    if(strcmp(ParticipantsTable[computed_hash]->Hostname,hostname) == 0)
        tmp=CreateCopyParticipant(ParticipantsTable[computed_hash]);
    else
        tmp= CreateCopyParticipant(find_in_next(ParticipantsTable[computed_hash]->next,hostname));        
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

struct sockaddr_in *getParticipantAddress(Participant *participant,int port)
{
    if(participant == NULL)
    {
        fprintf(participant_logfile,"Error on getting participant address, participant is null \n");
        return NULL;
    }
    if(participant->ip_address == NULL )
    {
        fprintf(participant_logfile,"Error on getting participant address, participant ip_address is null \n");
        return NULL;
    }
    struct sockaddr_in *serverAddr = (struct sockaddr_in *) malloc(sizeof(struct sockaddr_in));
    int worked = inet_aton(participant->ip_address,&serverAddr->sin_addr);
    fprintf(participant_logfile, "worked: %d", worked);
    fflush(participant_logfile);
    // if(worked != 0 && strcmp(participant->ip_address,"127.0.1.1"))
    // {
    //     fprintf(participant_logfile,"Error on getting participant address, participant ip_address is Invalid. Address: %s \n",participant->ip_address);
        
    //     return NULL;
    // }
    serverAddr->sin_family = AF_INET;
    serverAddr->sin_port = htons(port);
    return serverAddr;
}

void createMonitoringInfo(Participant *participant)
{
    MonitoringInfo *moni =(MonitoringInfo *) malloc(sizeof(MonitoringInfo));
    participant->monitoration= moni;
    moni->participant=CreateCopyParticipant(participant); //this is just so wrong
    moni->time_to_sleep=3;
    pthread_create(&moni->monitoringThread, NULL, monitorParticipant, moni);
}

void destroyMonitoringInfo(Participant *participant)
{

    pthread_cancel(participant->monitoration->monitoringThread);
    free(participant->monitoration);
}

#endif