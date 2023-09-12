#include "election.h"

FILE *election_logfile ;

#define PORT_CLIENT_ELECT 4031
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





int CallElection()
{
    int i, send_ret;
    int is_new_manager = 1;
    Participant *self = getMyselfCopy();
    List_Participant *List=getParticipant_list();


    for (i=0; i<LIST_SIZE; i++) {
        if(List->list[i].id != -1 && List->list[i].id > self->id) {
            is_new_manager = 0;
            //struct sockaddr_in *participantAddress = getParticipantAddress(monoration->participant,PORT_CLIENT_MON);
            //send_ret =sendto(threadPort, request, sizeof(List_Participant), 0,(struct sockaddr *) participantAddress, sizeof(struct sockaddr));
        }
    }

    return is_new_manager;
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
    if (bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        printf("Failed to bind the socket.\n");
        close(sockfd);
        raise(SIGINT);
    }
    struct timeval tv;

    return sockfd;
}