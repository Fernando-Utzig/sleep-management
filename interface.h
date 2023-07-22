#ifndef H_INTERFACE
#define H_INTERFACE
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include "participants.h"
#include <arpa/inet.h>
#include <signal.h>

void *interfaceThreadParticipant(void *arg);
void *interfaceThreadManager(void *arg);
#endif