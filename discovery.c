#ifndef C_DISCOVERY
#define C_DISCOVERY
#include "discovery.h"


int createSocket(int port, char serverName[]) {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    int yes = 1;
    int ret;
    struct hostent *server;
    if (sockfd < 0) {
        printf("Failed to create socket.");
        exit(EXIT_FAILURE);
    }
    ret = setsockopt(sockfd,SOL_SOCKET,SO_BROADCAST,&yes,sizeof(yes));
    if(ret != 0)
    {
        printf("Failed to configure the socket.");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    if(serverName == NULL)
        serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    else{
        printf("\ngetting host name %s\n", serverName);
        server = gethostbyname(serverName);
        if(server == NULL)
        {
            printf("failed to find host;\n");
        }
        serverAddr.sin_addr = *((struct in_addr *)server->h_addr);
    }
    bzero(&(serverAddr.sin_zero), 8); 
    if(serverName == NULL)
        if (bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
            printf("Failed to bind the socket.");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
    printf("Socket id = %d",sockfd);
    printf("\tDiscovery Socket Created\n");
    return sockfd;
}

void *discoveryThread(void *arg) {
    printf("Starting Discovery\n");
    int sockfd = createSocket(PORT,NULL);
    struct sockaddr_in clientAddr;
    char buffer[BUFFER_SIZE];
    socklen_t len = sizeof(clientAddr);
    int send_ret,n,insertion_result;
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&clientAddr, &len);
        fprintf(stderr," n= %d",n);
        fprintf(stderr,"no n \n");
        if (n > 0) {
            fprintf(stderr,"received packaged ");
            buffer[n] = '\0';
            fprintf(stderr,"message = %s\n",buffer);
            insertion_result=AddParticipantToTable(buffer,n);
            if(insertion_result<0)
                send_ret = sendto(sockfd, "FAILED TO ADD\n", strlen("FAILED TO ADD\n"), 0,(struct sockaddr *) &clientAddr, sizeof(struct sockaddr));
            else
                send_ret = sendto(sockfd, "You have been added Succesfully\n", strlen("You have been added Sucessfuly\n"), 0,(struct sockaddr *) &clientAddr, sizeof(struct sockaddr));
            fprintf(stderr,"send_ret = %d",send_ret);
        }
    }
}

int sendDiscoverypackaged(struct sockaddr_in *Manageraddress)
{
    unsigned int length;
    struct sockaddr_in serveraddress;
    FILE * eth0 = fopen("/sys/class/net/eth0/address", "r");
    char MyMac[20];
	if(eth0 == NULL)
	{
		printf("\nMY MAC READ FAILED- to open file\n");
        strcpy(MyMac,"AA:BB:CC:DD:EE:FF");
	}
    else
    {
        if(fgets(MyMac, 18, eth0) == NULL )// getting only the mac, which is the 18 first char
        {
            printf("\nMY MAC READ FAILED-to read file\n");
            strcpy(MyMac,"AA:BB:CC:DD:EE:FF");
        }
    }
    char MyHostName[256];
    if(gethostname(MyHostName,sizeof(MyHostName)) == -1)
    {
        printf("\nMY HOSTNAME READ FAILED");
        strcpy(MyHostName,"MyDefaultaHostname");
    }
	char sendMessage[BUFFER_SIZE];
    fprintf(stderr,"\nMyMac is =%s",MyMac);
    strcpy(sendMessage,MyMac);
    strcat(sendMessage, ",");
    fprintf(stderr,"\nhostname is = %s",MyHostName);
    strcat(sendMessage, MyHostName);
    fprintf(stderr,"\nsendmessage = %s", sendMessage);
    char buffer[BUFFER_SIZE];
    char serv_addr[] = "255.255.255.255"; // MY BROADCAST IP
    int send,receive;
    struct hostent *server;
    int sockfd = createSocket(PORT_CLIENT,serv_addr);
    length = sizeof(struct sockaddr_in);
    serveraddress.sin_family = AF_INET;     
	serveraddress.sin_port = htons(PORT);    
    server = gethostbyname(serv_addr);
    struct timeval tv;
    tv.tv_sec = 3;
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
    if(server == NULL)
    {
        fprintf(stderr,"problem finding master server");
    }
	serveraddress.sin_addr = *((struct in_addr *)server->h_addr);
    receive = 0; //Mudar para receber confirmacao
    int tries =0;
    do{
        fprintf(stderr,"\nsending discovery packaged");
        send = sendto(sockfd, sendMessage, strlen(sendMessage), 0, (const struct sockaddr *) &serveraddress, sizeof(struct sockaddr_in));
        fprintf(stderr,"\n send value = %d",receive);
        if (send < 0) 
            fprintf(stderr,"\nERROR sendto: %d \n",send);
        else
        {
            receive = recvfrom(sockfd, buffer, 256, 0, (struct sockaddr *) Manageraddress, &length);
            fprintf(stderr,"\nreceive value = %d",receive);
            fprintf(stderr,"\nreceived discovery packaged: %s",buffer);
        }
        tries++;
        tv.tv_usec = 0;
        
    }while((send <0 || receive <0) && tries <CONFIRMATION_TRIES);
    if(tries >CONFIRMATION_TRIES)
    {
        return -1;
    }
    return 0;
}

#endif
