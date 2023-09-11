#ifndef C_PARTICIPANTS
#define C_PARTICIPANTS
#define TABLE_SIZE 997

#include "participants.h"

Participant* ParticipantsTable[TABLE_SIZE];
List_Participant List;
pthread_mutex_t participantsMutex;
pthread_mutex_t writerIsqueueMutex;
pthread_mutex_t readersMutex;
Participant myself;
Participant *Manager;
pthread_mutex_t myselfMutex;
sem_t ReadersAreInRoom;
int readers;
Participant * CreateCopyParticipant(Participant *original);
void createMonitoringInfo(Participant *participant);
void destroyMonitoringInfo(Participant *participant);
void ListReaderEnterRoom();
void ListReaderLeaveRoom();

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
    ListReaderEnterRoom();
    printf("\n");
    printf("-------------------------\n");
    if(myself.is_manager)
        printf("         Manager        \n");
    else
        printf("        Participant        \n");
    printf("----------List-Version:%d---------\n",List.list_version);
    printf("Hostname| ID |  IP  |  MAC  |  Status\n");
    for (int i=0; i<LIST_SIZE; i++) {
        if(List.list[i].id>0)
            printParticipant(&List.list[i]);
    }
    ListReaderLeaveRoom();
}

void setManager(Participant *received)
{
    Manager = CreateCopyParticipant(received);
}

List_Participant* getParticipant_list()
{
    return &List;
}


void ListReaderEnterRoom()
{
    pthread_mutex_lock(&participantsMutex);
    pthread_mutex_unlock(&participantsMutex);
    pthread_mutex_lock(&readersMutex);
    readers+=1;
    if(readers==1)
        sem_wait(&ReadersAreInRoom);
    pthread_mutex_unlock(&readersMutex);
}

void ListReaderLeaveRoom()
{
    pthread_mutex_lock(&readersMutex);
    readers-=1;
    if(readers==0)
        sem_post(&ReadersAreInRoom);
    pthread_mutex_unlock(&readersMutex);
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
    if( ip == NULL)
    {
        fprintf(participant_logfile,"Tried to set NULL IP\n");
        return;
    }
    fprintf(participant_logfile,"my now ip is %s\n",ip);
    strcpy(myself.ip_address,ip);
    updateParticipant(&myself);
    return;
}

void setMySelfId(int id)
{
    pthread_mutex_lock(&myselfMutex);
    myself.id=id;
    pthread_mutex_unlock(&myselfMutex);
    updateParticipant(&myself);
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
        

    }
    myself.is_awaken = 1;
    myself.is_manager = 0;
    fprintf(participant_logfile,"MyMac is =%s\n",myself.MAC);
    fprintf(participant_logfile,"hostname is = %s\n",myself.Hostname);
    fprintf(participant_logfile,"myip is = %s\n",myself.ip_address);
}
void printManager()
{
    system("clear"); // Comando para limpar a tela no Ubuntu
    printf("\n");
    printf("-------------------------\n");
    printf("        Manager          \n");
    printf("-------------------------\n");
    printf("Hostname | ID |  IP  |  MAC  |  Status\n");
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

void init_participantList(void)
{
    int i;
    fprintf(participant_logfile,"Inicianting Participant List\n");
    fflush(participant_logfile);
    for(i=0;i<LIST_SIZE;i++)
    {
        List.list[i].id=-1;
        List.list[i].monitoration=NULL;
    }
    List.list_version=0;
    List.list_size=0;
    List.maxId=1;
    fprintf(participant_logfile,"participants List iniciated\n");
    readers=0;
    sem_init(&ReadersAreInRoom, 0, 1);
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
    new_participant->id=List.maxId;
    List.maxId++;
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
    printParticipant(new_participant);
    return new_participant;
}





Operation_result AddParticipantToTable(Participant *participant)
{
    Operation_result res;
    fprintf(participant_logfile,"Adding participant\n");
    if(participant == NULL)
    {
        fprintf(participant_logfile,"AddParticipantToTable participant is NULL\n");
        res.id=-1;
        res.result=-5;
        return res;
    }
    int i;
    int found=-1;
    pthread_mutex_lock(&participantsMutex);//block readers
    sem_wait(&ReadersAreInRoom);//wait until all readers leave
    for(i=0;i<LIST_SIZE;i++)
    {
        if(List.list[i].id != -1 && (List.list[i].id ==participant->id))//List.list[i].id != -1 && strcmp(List.list[i].Hostname,participant->Hostname) == 0)
        {
            res.id=List.list[i].id;
            found=i;
            break;
        }
    }
    if(found!=-1)
    {
        fprintf(participant_logfile,"achou mesmo id/hostname\n");
        fflush(participant_logfile);
        
        res.result= 2;
    }
    else
    {
        found=-1;
        for(i=0;i<LIST_SIZE;i++)
        {
            if(List.list[i].id == -1)
            {
                fprintf(participant_logfile,"Achou espaço! index = %d\n",i);
                found =i;
                break;
            }
        }
        if(found!=-1){
            participant->id=List.maxId;
            res.id=List.maxId;
            List.maxId++;
            List.list_version++;
            copyParticipant(&List.list[i],participant);
            if(List.list[i].is_manager==0)
                createMonitoringInfo(&List.list[i]);
            printf("\n novo id=%d \n",List.list[i].id);
            res.result=1;
        }
        else
        {
            res.result=-1;
        }
        
    }
    
    pthread_mutex_unlock(&participantsMutex);// now all readers/writers can enter
    sem_post(&ReadersAreInRoom);//room is empty
    fprintf(participant_logfile,"Adding participant result value : %d id; %d\n",res.result,res.id);
    fflush(participant_logfile);
    display();
    return res;
}



Operation_result updateParticipant(Participant *participant)
{
    Participant *tmp;
    Operation_result res;
    fprintf(participant_logfile,"Updating participant\n");
    fflush(participant_logfile);
    if(participant == NULL)
    {
        fprintf(participant_logfile,"Trying to update NULL participant\n");
        res.result=5;
        res.id=-1;
        return res;
    }
    int i, found= -1;
    pthread_mutex_lock(&participantsMutex);
    sem_wait(&ReadersAreInRoom);//wait until all readers leave
    fprintf(participant_logfile,"finding participant\n");
    fflush(participant_logfile);
    for(i=0;i<LIST_SIZE;i++)
    {
        if(List.list[i].id==participant->id)
        {
            found =i;
            fprintf(participant_logfile,"Found participant with id=%d index=%d\n",participant->id,i);
            break;
        }
    }
    if(found == -1)
    {
        res.result= -1;
        res.id=participant->id;;
        fprintf(participant_logfile,"Did not Found participant with id=%d \n",participant->id);
    }
    else
    {
        List.list_version++;
        copyParticipant(&List.list[i],participant);
        res.result=1;
        res.id=List.list[i].id;
    }
    pthread_mutex_unlock(&participantsMutex);
    sem_post(&ReadersAreInRoom);//room is empty
    display();
    return res;

}
Operation_result removeParticipantFromTable(Participant *participant)
{
    
    Participant *tmp,*tmp2;
    Operation_result res;
    if(participant == NULL)
    {
        fprintf(participant_logfile,"Trying to remove NULL participant\n");
        fflush(participant_logfile);
        res.result=-5;
        res.id=-1;
        return res;
    }
    fprintf(participant_logfile,"Removin Participant with id =%d\n",participant->id);
    int i, found =-1;
    res.id=participant->id;
    pthread_mutex_lock(&participantsMutex);
    sem_wait(&ReadersAreInRoom);//room is empty
        for(i=0;i<LIST_SIZE;i++)
        {
            if(List.list[i].id==participant->id)
            {
                if(List.list[i].is_manager==0)
                    destroyMonitoringInfo(&List.list[i]);
                List.list[i].id=-1;
                found=i;
                res.result=1;
                List.list_version++;
                break;
            }
        }
    pthread_mutex_unlock(&participantsMutex);
    sem_post(&ReadersAreInRoom);//room is empty
    if(found==-1)
        res.result=-1;
    display();
    fprintf(participant_logfile,"removing result: %d index found =%d\n",res.result,found);
    fflush(participant_logfile);
    return res;
}

void printParticipant(Participant *participant)
{
    if (participant == NULL)
        return;

    if (participant->Hostname != NULL)
        printf("%s | ", participant->Hostname);
    printf("%d | ", participant->id);
    if (participant->ip_address != NULL)
        printf("%s | ", participant->ip_address);
    if (participant->MAC != NULL)
        printf("%s | ", participant->MAC);
    if (participant->is_awaken == 0)
        printf("Sleeping");
    else
        printf("Active");
    if (participant->is_manager == 1)
        printf(" | Manager");
    printf("\n");
    printf("-------------------------\n");
}

void copyParticipant(Participant *copy,Participant *original)
{
    fprintf(participant_logfile,"copying participant\n");
    copy->id=original->id;
    strcpy(copy->Hostname,original->Hostname);
    strcpy(copy->ip_address,original->ip_address);
    strcpy(copy->MAC,original->MAC);
    copy->is_awaken=original->is_awaken;
    copy->is_manager=original->is_manager;
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
    Participant *tmp=NULL;
    if(hostname == NULL)
    {
        fprintf(participant_logfile,"ERROR Mac is null ");
        return NULL;
    }
    int i, found= -1;
    pthread_mutex_lock(&participantsMutex);
        for(i=0;i<LIST_SIZE;i++)
        {
            if(List.list[i].id != -1 && strcmp(List.list[i].Hostname,hostname) == 0)
            {
                tmp = CreateCopyParticipant(&List.list[i]);
                break;
            }
        }
    pthread_mutex_unlock(&participantsMutex);
    return tmp;
}

void setMyselfSleep()
{
    pthread_mutex_lock(&myselfMutex);
    myself.is_awaken=0;
    pthread_mutex_unlock(&myselfMutex);
}

void setMyselfAsManager()
{
    pthread_mutex_lock(&myselfMutex);
    myself.is_manager=1;
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
    serverAddr->sin_family = AF_INET;
    serverAddr->sin_port = htons(port);
    return serverAddr;
}

void createMonitoringInfo(Participant *participant)
{
    fprintf(participant_logfile,"creating MonitoringInfo for participant id: %d\n",participant->id);
    if(participant->is_manager==1)
    {
        fprintf(participant_logfile,"Can not create monitoringInfo for the manager\n");
        fflush(participant_logfile);
        return;
    }
    MonitoringInfo *moni =(MonitoringInfo *) malloc(sizeof(MonitoringInfo));
    participant->monitoration= moni;
    moni->participant=CreateCopyParticipant(participant); //this is just so wrong
    moni->time_to_sleep=3;
    pthread_create(&moni->monitoringThread, NULL, monitorParticipant, moni);
}

void destroyMonitoringInfo(Participant *participant)
{
    fprintf(participant_logfile,"Destroying Monitoring info Participant id: %d\n",participant->id);
    if(participant->monitoration == NULL)
    {
        fprintf(participant_logfile,"Participant monitoration is NULL, can not destroy\n");
        return;
    }
    
    pthread_cancel(participant->monitoration->monitoringThread);
    free(participant->monitoration);
}

void clearMonitoringInfo()
{//should only be used in getting newest list from manager, never inside manager. If necessary apply concurrent protection
    int i;
    for(i=0;i<LIST_SIZE;i++)
    {
        List.list[i].monitoration=NULL;
    }
}

void createAllMonitoringInfo()
{
    int i;
    for(i=0;i<LIST_SIZE;i++)
    {
        if(List.list[i].id !=-1){
            createMonitoringInfo(&List.list[i]);
        }
    }
}

void destroyAllMonitoringInfo()
{
    int i;
    for(i=0;i<LIST_SIZE;i++)
    {
        if(List.list[i].id !=-1){
            destroyMonitoringInfo(&List.list[i]);
        }
    }
}


#endif