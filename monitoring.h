
#ifndef H_MONITORING
#define H_MONITORING
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "participants.h"

struct ParticipantData typedef Participant;

void *ParticipantMonitoringThread(void *arg);
int sleepOrWakupParticipant(Participant *participant, int new_status);
void closeMonitoringSocket();
#endif