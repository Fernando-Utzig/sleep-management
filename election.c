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
    int i;
    char message[1024];
    strcpy(message, "ELECTION");
    int is_new_manager = 1;
    
    getMyselfCopy
    for (i=0; i<LIST_SIZE; i++) {
        if(List.list[i].id != -1 && List.list[i].id > ) {
            
        }
    }

    return is_new_manager;
}