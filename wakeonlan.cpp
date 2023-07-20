#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <iostream>
#include <vector>
#include "strucks.cpp"
using namespace std;

#define PORT 4000
#define PORT_CLIENT 4021
#define BUFFER_SIZE 1024
#define MONITORING_PORT 8889
#define CONFIRMATION_TRIES 3
struct sockaddr_in ManagerSock;
// Struct que armazena os dados de cada participante


// A tabela é representada por uma lista de structs
vector<ParticipantData> ParticipantsTable;

// Variáveis globais
bool isManager = false;
int numParticipants = 0;

pthread_mutex_t participantsMutex;
pthread_rwlock_t participantsRWLock;
pthread_mutex_t tableMutex = PTHREAD_MUTEX_INITIALIZER;


// Executado pelo manager
// Identifica quais computadores passaram a executar o programa, recebendo e respondendo pacotes em broadcast

int createSocket(int port, char serverName[]) {
        int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        int yes = 1;
        int ret;
        struct hostent *server;
        if (sockfd < 0) {
            std::cerr << "Failed to create socket." << std::endl;
            exit(EXIT_FAILURE);
        }

        ret = setsockopt(sockfd,SOL_SOCKET,SO_BROADCAST,&yes,sizeof(yes));
        if(ret != 0)
        {
            std::cerr << "Failed to configure the socket." << std::endl;
            exit(EXIT_FAILURE);
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
        if(serverName == NULL)
            if (bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
                std::cerr << "Failed to bind the socket." << std::endl;
                close(sockfd);
                exit(EXIT_FAILURE);
            }
        printf("Socket id = %d",sockfd);
        std::cout << "\tDiscovery Socket Created\n" << std::endl;
        return sockfd;
    }

void *discoveryThread(void *arg) {
        std::cout << "Starting Discovery\n" << std::endl;
        int sockfd = createSocket(PORT,NULL);
        SMetaData *metadata = (SMetaData *) arg;
        struct sockaddr_in clientAddr;
        char buffer[BUFFER_SIZE];
        socklen_t len = sizeof(clientAddr);
        int send_ret;
        while (1) {
            memset(buffer, 0, BUFFER_SIZE);
            int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&clientAddr, &len);
            printf(" n= %d",n);
            std::cout << "no n \n" << std::endl;
            if (n > 0) {
                std::cout << "received packaged" << std::endl;
                buffer[n] = '\0';
                printf("message = %s",buffer);
                // Extract the hostname from the buffer
                char* hostname = strtok(buffer, ",");
                if (hostname != NULL) {
                    // Extract the status from the buffer
                    printf("hostname received = %s\n",hostname);
                }
                send_ret = sendto(sockfd, "You have been added\n", 17, 0,(struct sockaddr *) &clientAddr, sizeof(struct sockaddr));
                printf("send_ret = %d",send_ret);
            }
        }
    }

int sendDiscoverypackaged(struct sockaddr_in *Manageraddress)
{
    unsigned int length;
    struct sockaddr_in serveraddress;
    FILE * eth0 = fopen("/sys/class/net/eth0/address", "r");
	if(eth0 == NULL)
	{
		printf("MY MAC READ FAILED");
	}
	char MyMac[19];
	fgets(MyMac, 19, eth0);
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
        printf("problem finding master server");
    }
	serveraddress.sin_addr = *((struct in_addr *)server->h_addr);
    receive = 0; //Mudar para receber confirmacao
    int tries =0;
    do{
        printf("\nsending discovery packaged");
        send = sendto(sockfd, MyMac, strlen(MyMac), 0, (const struct sockaddr *) &serveraddress, sizeof(struct sockaddr_in));
        printf("\n send value = %d",receive);
        if (send < 0) 
            printf("\nERROR sendto: %d \n",send);
        else
        {
            receive = recvfrom(sockfd, buffer, 256, 0, (struct sockaddr *) Manageraddress, &length);
            printf("\nreceive value = %d",receive);
            printf("\nreceived discovery packaged: %s",buffer);
        }
        tries++;
        tv.tv_usec = 0;
        
    }while((send <0 && receive <0)|| tries <CONFIRMATION_TRIES);
    if(tries >CONFIRMATION_TRIES)
    {
        return -1;
    }
    return 0;
}
// Executado pelo manager
// Monitora o status dos computadores a partir do envio de pacotes

void* monitoringService(void* arg) {
    int sockfd = createSocket(MONITORING_PORT,NULL);
    struct sockaddr_in clientAddr;
    char buffer[BUFFER_SIZE];
    while (1) {

    }
}

// Executado pelo manager
// Mantém uma lista dos computadores participantes com o status de cada um


// Executado pelo manager e pelo participante
// Exibe informações dos computadores e permite input dos usuários
class interface_subservice{
public:

    // Função para exibir a lista de participantes na tela
    void displayParticipants() {
        pthread_mutex_lock(&tableMutex);
        std::cout << "Participants:" <<  std::endl;
        for (int i=0; i<ParticipantsTable.size(); i++) {
            std::cout << "Hostname: " << ParticipantsTable[i].Hostname << ", IP: " << ParticipantsTable[i].ip_address
                      << ", MAC: " << ParticipantsTable[i].MAC << ", Status: " << ParticipantsTable[i].is_awaken << std::endl;
        }
        pthread_mutex_unlock(&tableMutex);
    }


    void    interfaceThread(void *arg) {
        std::string command;
        std::cout << "Digite o comando: " << std::endl;


        while (1) {
            std::cin >> command;

            if (command == "EXIT") {
                if (isManager) {
                    pthread_rwlock_wrlock(&participantsRWLock);
                    std::cout << "Estação Manager está saindo do serviço" << std::endl;
                    pthread_rwlock_unlock(&participantsRWLock);
                } else {
                    std::cout << "Enviando mensagem de saída..." << std::endl;
                    // Enviar pacote de descoberta especial para indicar saída do serviço
                }

                break;
            } else if (command == "WAKEUP") {
                std::string hostname;
                std::cin >> hostname;
                std::cout << "Enviando comando de WAKEUP para a estação" << hostname << std::endl;
                // Enviar comando de WAKEUP (pacote WoL) para a estação especificada
            } else {
                std::cout << "Comando inválido." << std::endl;
            }
        }
    }
};


int main(int argc, char *argv[]){

    pthread_t discoveryThreadId, interfaceThreadId, monitoringThreadId, managementThreadId;
    SMetaData *metadata = (SMetaData *)malloc(sizeof(SMetaData));
    if (argc > 1 && strcmp(argv[1], "manager") == 0) {
        isManager = true;
        std::cout << "Estação iniciada como Manager\n" << std::endl;
        pthread_create(&discoveryThreadId, NULL, discoveryThread, metadata);
        while(1)
        {

        }
    } else {
            std::cout << "Estação iniciada como Participante\n" << std::endl;
            std::string hostname = "MyComputer";
            std::string mac = "AA:BB:CC:DD:EE:FF";
            std::string ip = "192.168.1.100";
            bool is_awaken = true;
            if(sendDiscoverypackaged(&ManagerSock) == -1)
            {
                printf("\nFailed to discover Manager, Closing");
                exit(1);
            }
            // Adicionar o participante na tabela


            // Mostrar participantes
            interface_subservice interface;

            interface.displayParticipants();
    }

    // Adicionar na tabela o novo participante
    //
 
};