#ifndef H_DISCOVERY
#define H_DISCOVERY
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

#define PORT 4000
#define BUFFER_SIZE 1024
#define PORT_CLIENT 4021
#define CONFIRMATION_TRIES 3
int sendDiscoverypackaged(struct sockaddr_in *Manageraddress);
void *discoveryThread(void *arg);
int sendDiscoverypackaged(struct sockaddr_in *Manageraddress);
int createSocket(int port, char serverName[]);
#endif