#ifndef C_DISCOVERY
#define C_DISCOVERY
#include "discovery.h"
#define CONFIRMATION_TRIES 3

int MySocket =-1;
void createDiscoveryPackage(char * sendMessage);

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
    fprintf(stderr,"Socket id = %d",sockfd);
    fprintf(stderr,"\tDiscovery Socket Created\n");
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
    fprintf(stderr,"Starting Discovery\n");
    int sockfd = createSocket(PORT,NULL);
    if(sockfd == -1)
        return NULL;
    struct sockaddr_in clientAddr;
    char buffer[BUFFER_SIZE];
    char managerInfo[BUFFER_SIZE];
    createDiscoveryPackage(managerInfo);
    socklen_t len = sizeof(clientAddr);
    int send_ret,n,insertion_result;
    int teste;
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
                send_ret = sendto(sockfd, managerInfo, strlen(managerInfo), 0,(struct sockaddr *) &clientAddr, sizeof(struct sockaddr));
            fprintf(stderr,"send_ret = %d\n",send_ret);
        }
        fflush(stderr);
    }
}

void getMyMac(char *myMac)
{
    if(myMac == NULL)
    {
        printf("Trying to get mac with NULL string\n");
        return;
    }
        
    DIR *dp;
    struct dirent *ep;
    char address_path[256] = "/sys/class/net/";
    dp = opendir ("/sys/class/net/");
    if (dp == NULL)
    {
        printf("MY MAC READ FAILED- failed to read directory /sys/class/net/\n");
        strcpy(myMac,"AA:BB:CC:DD:EE:FF");
        return;
    }
    do
    {
        ep = readdir (dp);
    } while (ep != NULL && ep->d_name[0]=='.');
    if(ep == NULL)
    {
        printf("MY MAC READ FAILED- failed to get connection folder in /sys/class/net/\n");
        strcpy(myMac,"AA:BB:CC:DD:EE:FF");
        return;
    }
    
    fprintf(stderr,"ep = %s\n",ep->d_name);
    strcat(address_path,ep->d_name);
    strcat(address_path,"/address");
    fprintf(stderr,"address path used = %s\n",address_path);
    FILE * eth0 = fopen("/sys/class/net/enp8s0/address", "r");
	if(eth0 == NULL)
	{
		printf("MY MAC READ FAILED- to open file\n");
        strcpy(myMac,"AA:BB:CC:DD:EE:FF");
        return;
	}

    if(fgets(myMac, 18, eth0) == NULL )// getting only the mac, which is the 18 first char
    {
        printf("MY MAC READ FAILED-to read file\n");
        strcpy(myMac,"AA:BB:CC:DD:EE:FF");
    }
    printf("mymac = %s\n",myMac);
    return;
}

void createDiscoveryPackage(char * sendMessage)
{
    struct hostent *myhost;
    char *myip;
    char myMac[256];
    if(sendMessage == NULL)
    {
        fprintf(stderr,"createDiscoveryPackage got a NULL sendMessage\n");
    }
    getMyMac(myMac);
    char MyHostName[256];
    if(gethostname(MyHostName,sizeof(MyHostName)) == -1)
    {
        printf("MY HOSTNAME READ FAILED\n");
        strcpy(MyHostName,"MyDefaultaHostname");
        myip = "10.1.1.1";
    }
    else
    {
        myhost = gethostbyname(MyHostName);
        myip = inet_ntoa(*((struct in_addr*)
                        myhost->h_addr_list[0]));
    }
	
    
    fprintf(stderr,"MyMac is =%s\n",myMac);
    strcpy(sendMessage,myMac);
    strcat(sendMessage, ",");
    fprintf(stderr,"hostname is = %s\n",MyHostName);
    strcat(sendMessage, MyHostName);
    fprintf(stderr,"myip is = %s\n",myip);
    strcat(sendMessage, ",");
    strcat(sendMessage, myip);
    fprintf(stderr,"sendmessage = %s\n", sendMessage);
}

int sendDiscoverypackaged(struct sockaddr_in *Manageraddress)
{
    unsigned int length;
    struct sockaddr_in serveraddress;
    
    
    char sendMessage[BUFFER_SIZE];
    createDiscoveryPackage(sendMessage);
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
    tv.tv_sec = 10;
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
    if(server == NULL)
    {
        fprintf(stderr,"problem finding master server\n");
    }
	serveraddress.sin_addr = *((struct in_addr *)server->h_addr);
    receive = 0; //Mudar para receber confirmacao
    int tries =0;
    struct in_addr ip_addr;
    char *some_addr;
    do{
        fprintf(stderr,"sending discovery packaged\n");
        send = sendto(sockfd, sendMessage, strlen(sendMessage), 0, (const struct sockaddr *) &serveraddress, sizeof(struct sockaddr_in));
        fprintf(stderr," send value = %d\n",receive);
        if (send < 0) 
            fprintf(stderr,"ERROR sendto: %d \n",send);
        else
        {
            receive = recvfrom(sockfd, buffer, 256, 0, (struct sockaddr *) Manageraddress, &length);
            fprintf(stderr,"receive value = %d\n",receive);
            fprintf(stderr,"received discovery packaged: %s\n",buffer);
            fprintf(stderr," Manager ip(in integer)= %u\n",Manageraddress->sin_addr.s_addr);
            fprintf(stderr," Manager ip(in string)= %s\n",inet_ntoa(Manageraddress->sin_addr));
            setManager(buffer,receive);
        }
        tries++;
        tv.tv_usec = 0;
        
    }while((send <0 || receive <0) && tries <CONFIRMATION_TRIES);
    if(tries >=CONFIRMATION_TRIES)
    {
        return -1;
    }
    return 0;
}

void createExitPackage(char * sendMessage)
{
    struct hostent *myhost;
    char *myip;
    char myMac[256];
    if(sendMessage == NULL)
    {
        fprintf(stderr,"createExitPackage got a NULL sendMessage\n");
    }
    getMyMac(myMac);
    char MyHostName[256];
    if(gethostname(MyHostName,sizeof(MyHostName)) == -1)
    {
        printf("MY HOSTNAME READ FAILED\n");
        strcpy(MyHostName,"MyDefaultaHostname");
        myip = "10.1.1.1";
    }
    else
    {
        myhost = gethostbyname(MyHostName);
        myip = inet_ntoa(*((struct in_addr*)
                        myhost->h_addr_list[0]));
    }
	
    // Concatenando o MAC, Hostname e Ip ao sendMassage
    fprintf(stderr,"MyMac is =%s\n",myMac);
    strcpy(sendMessage,myMac);
    strcat(sendMessage, ",");
    fprintf(stderr,"hostname is = %s\n",MyHostName);
    strcat(sendMessage, MyHostName);
    fprintf(stderr,"myip is = %s\n",myip);
    strcat(sendMessage, ",");
    strcat(sendMessage, myip);
    fprintf(stderr,"sendmessage = %s\n", sendMessage);
}

int sendExitPacket(struct sockaddr_in *Manageraddress)
{
    unsigned int length;
    struct sockaddr_in serveraddress;
    
    
    char sendMessage[BUFFER_SIZE];
    createExitPackage(sendMessage);
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
    tv.tv_sec = 10;
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
    if(server == NULL)
    {
        fprintf(stderr,"problem finding master server\n");
    }
	serveraddress.sin_addr = *((struct in_addr *)server->h_addr);
    receive = 0; //Mudar para receber confirmacao
    int tries =0;
    struct in_addr ip_addr;
    char *some_addr;
    do{
        fprintf(stderr,"sending sleep service exit packaged\n");
        send = sendto(sockfd, sendMessage, strlen(sendMessage), 0, (const struct sockaddr *) &serveraddress, sizeof(struct sockaddr_in));
        fprintf(stderr," send value = %d\n",receive);
        if (send < 0) 
            fprintf(stderr,"ERROR sendto: %d \n",send);
        else
        {
            receive = recvfrom(sockfd, buffer, 256, 0, (struct sockaddr *) Manageraddress, &length);
            fprintf(stderr,"receive value = %d\n",receive);
            fprintf(stderr,"received discovery packaged: %s\n",buffer);
            fprintf(stderr," Manager ip(in integer)= %u\n",Manageraddress->sin_addr.s_addr);
            fprintf(stderr," Manager ip(in string)= %s\n",inet_ntoa(Manageraddress->sin_addr));
        }
        tries++;
        tv.tv_usec = 0;
        
    }while((send <0 || receive <0) && tries <CONFIRMATION_TRIES);
    if(tries >=CONFIRMATION_TRIES)
    {
        return -1;
    }
    return 0;
}

#endif
