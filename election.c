#include "election.h"

struct struct_election
{
    int election_command;
} typedef structElection;

int CallElection()
{
    int i;
    int is_new_manager = 1;
    Participant *self = getMyselfCopy();
    List_Participant *List=getParticipant_list();



    
    for (i=0; i<LIST_SIZE; i++) {
        if(List->list[i].id != -1 && List->list[i].id > self->id) {
            
        }
    }

    return is_new_manager;
}