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
#include "discovery.h"
#include "monitoring.h"




#define MONITORING_PORT 8889

#define PARTICIPANT_TABLE_SIZE 64
struct sockaddr_in ManagerSock;
// Struct que armazena os dados de cada participante

// Variáveis globais
int isManager = 0;
int numParticipants = 0;


pthread_mutex_t  participantsRWLock;
pthread_mutex_t tableMutex = PTHREAD_MUTEX_INITIALIZER;


// Executado pelo manager
// Identifica quais computadores passaram a executar o programa, recebendo e respondendo pacotes em broadcast



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

// Função para exibir a lista de participantes na tela
void displayParticipants() {
    pthread_mutex_lock(&tableMutex);
    printf("Participants:");
    printAllParticipants();
    pthread_mutex_unlock(&tableMutex);
}


void interfaceThread(void *arg) {
    char command[100];
    while (1) {
        
        scanf("Commands: EXIT and WAKEUP:\n %s",command);
        if (command == "EXIT") {
            if (isManager) {
                pthread_mutex_unlock(&participantsRWLock);
                pthread_mutex_unlock(&participantsRWLock);
            } else {
                // Enviar pacote de descoberta especial para indicar saída do serviço
            }
            break;
        } else if (command == "WAKEUP") {
            // Enviar comando de WAKEUP (pacote WoL) para a estação especificada
        } else {
            printf("Comando inválido.");
        }
    }
}



int main(int argc, char *argv[]){

    pthread_t discoveryThreadId, interfaceThreadId, monitoringThreadId, managementThreadId;
    init_participantTable();
    if (argc > 1 && strcmp(argv[1], "manager") == 0) {
        isManager = 1;
        printf("Estação iniciada como Manager\n");
        pthread_create(&discoveryThreadId, NULL, discoveryThread, NULL);
        while(1)
        {

        }
    } else {
        int is_awaken = 1;
        if(sendDiscoverypackaged(&ManagerSock) == -1)
        {
            printf("\nFailed to discover Manager, Closing");
            exit(1);
        }
        pthread_create(&monitoringThreadId, NULL, ParticipantMonitoringThread, &ManagerSock);
        
        while(1)
        {

        }
        displayParticipants();
    }

    // Adicionar na tabela o novo participante
    //
 
};