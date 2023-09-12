#include "election.h"

FILE *election_logfile ;

#define PORT_CLIENT_RECEIVE_ELECT 34134
#define PORT_CLIENT_SEND_ELECTION 34133

#define ELECTION 1
#define ELECTED 2
#define OK 3

int createSocketElection(int port);

void setElectionLogFile(FILE *file)
{
    if(file != NULL)
        election_logfile =file;
    else
    {
        election_logfile=stderr;
        fprintf(election_logfile,"Election got a null file, using std err instead \n");
    }
    fprintf(election_logfile,"election_logfile set\n");
}

struct struct_election
{
    int election_command;
} typedef structElection;



int election_is_happening;
int binded=0;
pthread_mutex_t election_is_happeningMutex;
pthread_t ElectionThread =0;
void *CallElection(void *arg)
{
    fprintf(election_logfile,"CallElection called \n");
    
    int i, send_ret, recv_ret;
    int is_new_manager = 0;
    Participant *self = getMyselfCopy();
    List_Participant *List=getParticipant_list();
    struct sockaddr_in *participantAddress;
    socklen_t len = sizeof(struct sockaddr_in);
    int do_election;
    structElection receive;
    structElection send;
    pthread_mutex_lock(&election_is_happeningMutex);
    int sockfd;
    if(election_is_happening == 1)
        do_election=0;
    else
    {
        do_election=1;
        election_is_happening=1;
        if(binded==0)
        {
            sockfd = createSocketElection(PORT_CLIENT_SEND_ELECTION);
            struct timeval tv;
            tv.tv_sec = 3;
            tv.tv_usec = 0;
            setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
            binded=sockfd;
        }

    }
    pthread_mutex_unlock(&election_is_happeningMutex);
    sockfd =binded;
    send.election_command=ELECTION;
    int responses=0;
    if(do_election==0)
        return NULL;
    fprintf(election_logfile,"Election Started \n");
    sockfd =binded;

    for (i=0; i<LIST_SIZE; i++) {
        if(List->list[i].id != -1 && List->list[i].id < self->id) {
            fprintf(election_logfile,"Sending to %d \n",List->list[i].id);
            fflush(election_logfile);
            participantAddress = getParticipantAddress(&List->list[i],PORT_CLIENT_RECEIVE_ELECT);
            send_ret =sendto(sockfd, &send, sizeof(List_Participant), 0,(struct sockaddr *) participantAddress, sizeof(struct sockaddr));
            recv_ret =recvfrom(sockfd, &receive, sizeof(List_Participant), 0,(struct sockaddr *) participantAddress, &len);
            if (recv_ret > 0)
            {
                responses++;
            }
        }
    }
    fprintf(election_logfile,"responses received %d \n",responses);
    fflush(election_logfile);
    if(responses==0)
    {
        is_new_manager=1;
        send.election_command=ELECTED;
        for (i=0; i<LIST_SIZE; i++) {
            if(List->list[i].id != -1) {
                participantAddress = getParticipantAddress(&List->list[i],PORT_CLIENT_SEND_ELECTION);
                send_ret =sendto(sockfd, &send, sizeof(List_Participant), 0,(struct sockaddr *) participantAddress, sizeof(struct sockaddr));
            }
        }
    }
    else
    {
        recvfrom(sockfd, &receive, sizeof(List_Participant), 0,(struct sockaddr *) participantAddress, &len);
        
        fprintf(election_logfile,"Received ELECTED message (%d): %d\n",ELECTED,receive.election_command);
    }
    pthread_mutex_lock(&election_is_happeningMutex);
    election_is_happening = 0;
    pthread_mutex_unlock(&election_is_happeningMutex);
    if(is_new_manager==1)
        raise(SIGUSR1);
    return NULL;
}

int createSocketElection(int port) {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    struct hostent *server;
    if (sockfd < 0) {
        printf("Failed to create socket.\n");
        raise(SIGINT);
    }
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    
    bzero(&(serverAddr.sin_zero), 8); 
    int bind_ret =bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    if (bind_ret < 0) {
        fprintf(election_logfile,"Failed to bind ELECTION the socket(port %d).bind_ret: %d errno %d\n",port,bind_ret,errno);
        close(sockfd);
        raise(SIGINT);
    }
    struct timeval tv;
    fprintf(election_logfile,"ELECTION socket created and binded port= %d\n",port);
    fflush(election_logfile);
    return sockfd;
}

void *RecieveElectionMessageThread(void *arg) {
    struct sockaddr_in clientAddr;
    int sockfd = createSocketElection(PORT_CLIENT_RECEIVE_ELECT);
    fprintf(election_logfile,"Port Created\n");
    structElection receive;
    structElection send;
    socklen_t len = sizeof(struct sockaddr_in);
    int n;
    int send_ret;
    while (1) {
        n = recvfrom(sockfd, &receive,  sizeof(structElection), 0, (struct sockaddr*)&clientAddr, &len);
        if(n!=-1)
            fprintf(election_logfile," n= %d\n",n);
        if (n > 0) {      
            switch (receive.election_command)
            {
            case ELECTION:
                send.election_command=OK;
                send_ret =sendto(sockfd, &send, sizeof(structElection), 0,(struct sockaddr*)&clientAddr, len);
                pthread_mutex_lock(&election_is_happeningMutex);
                if(election_is_happening == 0)
                {
                    pthread_create(&ElectionThread, NULL, CallElection, NULL);
                }
                pthread_mutex_unlock(&election_is_happeningMutex);
                break;
            case ELECTED:
                /* code */
                break;
            default:
                break;
            }
        }
        fflush(election_logfile);
    }
    return NULL;
}
