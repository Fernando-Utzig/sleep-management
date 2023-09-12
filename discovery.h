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
#include <net/if.h>
#include <sys/ioctl.h>

#define PORT 34000
#define BUFFER_SIZE 1024
#define PORT_CLIENT 34030
#define PORT_DISCOVER_MANAGER 34030
void setDiscoveryLogFile(FILE *file);
int sendDiscoverypackaged(struct sockaddr_in *Manageraddress);
void *discoveryThread(void *arg);
int sendDiscoverypackaged(struct sockaddr_in *Manageraddress);
int sendExitRequest(Participant *manager);
int createSocket(int port, char serverName[]);
void closeDiscoverySocket();
void *discoveryManagerThread(void *arg);
#endif