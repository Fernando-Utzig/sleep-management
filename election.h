#ifndef H_ELECTION
#define H_ELECTION
#include "participants.h"
#include <errno.h>
void * CallElection(void *arg);
void setElectionLogFile(FILE *file);
void *RecieveElectionMessageThread(void *arg);
#endif