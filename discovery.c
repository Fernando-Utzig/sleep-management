#ifndef C_DISCOVERY
#define C_DISCOVERY
#include "discovery.h"
#define CONFIRMATION_TRIES 3

int MySocket =-1;


#define NEW_ENTRY_COMMAND 1
#define EXIT_PARTICIPANT_COMMAND 2

typedef struct discovery_package_struct
{
    int command;
    Participant part;
    int result;
} discovery_package;

discovery_package *createDiscoveryPackage(int command);

FILE *discovery_logfile ;

void setDiscoveryLogFile(FILE *file)
{
    if(file != NULL) {
        printf("discovery log file set\n");
        discovery_logfile =file;
    }
        
    else
    {
        discovery_logfile=stderr;
        fprintf(discovery_logfile,"discovery got a null file, using stderr instead \n");
    }
    fprintf(discovery_logfile,"discovery_logfile set\n");
}

int createSocket(int port, char serverName[]) {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    int yes = 1;
    int ret;
    struct hostent *server;
    if (sockfd < 0) {
        printf("Failed to create socket.");
        raise(SIGINT);
    }
    ret = setsockopt(sockfd,SOL_SOCKET,SO_BROADCAST,&yes,sizeof(yes));
    if(ret != 0)
    {
        printf("Failed to configure the socket.");
        raise(SIGINT);
    }
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    if(serverName == NULL)
        serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    else{
        printf("getting host name %s\n", serverName);
        server = gethostbyname(serverName);
        if(server == NULL)
        {
            printf("failed to find host;\n");
        }
        serverAddr.sin_addr = *((struct in_addr *)server->h_addr);
    }
    bzero(&(serverAddr.sin_zero), 8); 
    if(serverName == NULL){
        int bind_ret = bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
        if (bind_ret < 0) {
            printf("Failed to bind the Discovery socket. bind_ret: %d\n",bind_ret);
            close(sockfd);
            raise(SIGINT);
            return -1;
        }
        else
            MySocket =sockfd;
    }
    fprintf(discovery_logfile,"Socket id = %d",sockfd);
    fprintf(discovery_logfile,"\tDiscovery Socket Created\n");
    return sockfd;
}

void closeDiscoverySocket()
{
    if(MySocket != -1)
        close(MySocket);
    return;
}

//struct sockaddr*copyClientAddress(struct sockaddr*client) //in case we need a another thread to proccess discovery packages
//{
//    struct sockaddr* copy = (struct sockaddr*) malloc(sizeof(struct sockaddr));
//    copy->sin_family=client->sin_family;
//    copy->sin_port=client->sin_port;
//    copy->sin_addr.s_addr=client->sin_addr.s_addr;
//    int i;
//    for(i=0;i<8;i++)
//        copy->sin_zero[i]=client->sin_zero[i];
//    return copy;
//}

void *discoveryThread(void *arg) {
    fprintf(discovery_logfile,"Starting Discovery\n");
    int sockfd = createSocket(PORT,NULL);
    if(sockfd == -1)
        return NULL;
    struct sockaddr_in clientAddr;
    discovery_package received_package;
    discovery_package *send_package;
    //char ip_from[BUFFER_SIZE];
    char *ip_rev;
    createDiscoveryPackage(NEW_ENTRY_COMMAND);
    socklen_t len = sizeof(clientAddr);
    int send_ret,n,operation_result;
    int teste;
    while (1) {
        n = recvfrom(sockfd, &received_package, sizeof(discovery_package), 0, (struct sockaddr*)&clientAddr, &len);
        ip_rev = inet_ntoa(clientAddr.sin_addr);
        printf("********\nip_rev = %s",ip_rev);
        strcpy(received_package.part.ip_address,ip_rev);
        fprintf(discovery_logfile," n= %d",n);
        fprintf(discovery_logfile,"no n \n");
        if (n > 0) {
            fprintf(discovery_logfile,"received packaged \n");
            fprintf(discovery_logfile,"command received: %d Mac:%s\n",received_package.command,received_package.part.MAC);
            if(received_package.command == NEW_ENTRY_COMMAND)
                operation_result=AddParticipantToTable(&received_package.part);
            if(received_package.command == EXIT_PARTICIPANT_COMMAND)
                operation_result=removeParticipantFromTable(&received_package.part);
            send_package = createDiscoveryPackage(received_package.command);
            send_package->result = operation_result;
            fprintf(discovery_logfile,"operation_result = %d\n",operation_result);
            send_ret = sendto(sockfd, send_package, sizeof(discovery_package), 0,(struct sockaddr *) &clientAddr, sizeof(struct sockaddr));
            free(send_package);
            fprintf(discovery_logfile,"send_ret = %d\n",send_ret);
        }
        fflush(discovery_logfile);
    }
}

discovery_package *createDiscoveryPackage(int command)
{
    fprintf(discovery_logfile,"creating discovery packaged\n");
    discovery_package *packaged = (discovery_package *) malloc(sizeof(discovery_package));
    packaged->command = command;
    packaged->result =0;
    Participant *mySelfCopy = getMyselfCopy();
    copyParticipant(&packaged->part,mySelfCopy);
    free(mySelfCopy);
    return packaged;
}

int sendDiscoverypackaged(struct sockaddr_in *Manageraddress)
{
    unsigned int length;
    struct sockaddr_in serveraddress;
    discovery_package *send_packaged =createDiscoveryPackage(NEW_ENTRY_COMMAND);
    discovery_package received_packaged;
    char serv_addr[] = "255.255.255.255"; // MY BROADCAST IP
    int send,receive;
    struct hostent *server;
    MySocket = createSocket(PORT_CLIENT,serv_addr);
    length = sizeof(struct sockaddr_in);
    serveraddress.sin_family = AF_INET;     
	serveraddress.sin_port = htons(PORT);    
    server = gethostbyname(serv_addr);
    struct timeval tv;
    tv.tv_sec = 10;
    tv.tv_usec = 0;
    setsockopt(MySocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
    if(server == NULL)
    {
        fprintf(discovery_logfile,"problem finding master server\n");
    }
	serveraddress.sin_addr = *((struct in_addr *)server->h_addr);
    receive = 0; //Mudar para receber confirmacao
    int tries =0;
    struct in_addr ip_addr;
    char *some_addr;
    do{
        fprintf(discovery_logfile,"sending discovery package\n");
        send = sendto(MySocket, send_packaged, sizeof(discovery_package), 0, (const struct sockaddr *) &serveraddress, sizeof(struct sockaddr_in));
        fprintf(discovery_logfile," send value = %d\n",receive);
        if (send < 0) 
            fprintf(discovery_logfile,"ERROR sendto: %d \n",send);
        else
        {
            receive = recvfrom(MySocket, &received_packaged, sizeof(discovery_package), 0, (struct sockaddr *) Manageraddress, &length);
            fprintf(discovery_logfile,"receive command: %d result: %d manager.mac= %s\n",received_packaged.command,received_packaged.result,received_packaged.part.MAC);
            fprintf(discovery_logfile," Manager ip(in integer)= %u\n",Manageraddress->sin_addr.s_addr);
            some_addr = inet_ntoa(Manageraddress->sin_addr);
            fprintf(discovery_logfile," Manager ip(in string)= %s\n",some_addr);
            strcpy(received_packaged.part.ip_address,some_addr);
            setManager(&received_packaged.part);
            fflush(discovery_logfile);
        }
        tries++;
        tv.tv_usec = 0;
        
    }while((send <0 || receive <0) && tries <CONFIRMATION_TRIES);
    free(send_packaged);
    if(tries >=CONFIRMATION_TRIES)
    {
        return -1;
    }
    return 0;
}

int sendExitRequest(Participant *manager)
{
    fprintf(discovery_logfile,"in sendExitRequest\n");
    fflush(discovery_logfile);
    int send_ret,receive_ret, request_result;
    discovery_package *send_packaged =createDiscoveryPackage(EXIT_PARTICIPANT_COMMAND);
    discovery_package received_packaged;
    int tries =0;
    struct sockaddr_in *managerAddress = getParticipantAddress(manager,PORT);
    
    if(managerAddress == NULL)
    {
        fprintf(discovery_logfile,"Failed to send request: managerAddress is NULL\n");
        return -1;
    }
    socklen_t len = sizeof(struct sockaddr_in);
    if(send_packaged == NULL)
    {
        fprintf(discovery_logfile,"Failed to send send_packaged: send_packaged is NULL\n");
        return -1;
    }
    fprintf(discovery_logfile,"exit sockfd = %d\n",MySocket);
    struct timeval tv;
    tv.tv_sec = 10;
    tv.tv_usec = 0;    
    fprintf(discovery_logfile,"setsockopt: %d errno: %d\n",setsockopt(MySocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv),errno);
    do
    {
        fprintf(discovery_logfile,"Sending EXIT Request try(%d)\n",tries);
        send_ret =sendto(MySocket, send_packaged, sizeof(discovery_package), 0,(struct sockaddr *) managerAddress, sizeof(struct sockaddr));
        if (send_ret < 0) 
            fprintf(discovery_logfile,"ERROR sendto: %d \n",send_ret);
        else
        {
            receive_ret = recvfrom(MySocket, &received_packaged, sizeof(discovery_package), 0, (struct sockaddr *) managerAddress, &len);
        }
        
        tv.tv_usec = 0;
        tries++;
    } while((send_ret <0 || receive_ret <0) && tries <CONFIRMATION_TRIES);
    if(tries >=CONFIRMATION_TRIES)
        request_result = -1;
    else
        request_result =1;
    
    free(send_packaged);
    return request_result;
}

#endif
