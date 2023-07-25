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
#include <arpa/inet.h>
#include <signal.h>
#include "participants.h"
#include <dirent.h>
#include "monitoring.h"
#include <errno.h>

#define PORT 4000
#define BUFFER_SIZE 1024
#define PORT_CLIENT 4026

int sendDiscoverypackaged(struct sockaddr_in *Manageraddress);
void *discoveryThread(void *arg);
int sendDiscoverypackaged(struct sockaddr_in *Manageraddress);
int sendExitRequest(Participant *manager);
int createSocket(int port, char serverName[]);
void closeDiscoverySocket();
#endif