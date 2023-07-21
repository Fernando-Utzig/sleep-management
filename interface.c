#ifndef C_INTERFACE
#define C_INTERFACE
#include "interface.h"

pthread_mutex_t tableMutex = PTHREAD_MUTEX_INITIALIZER;

void displayParticipants() {
    pthread_mutex_lock(&tableMutex);
    printf("Participants:");
    printAllParticipants();
    pthread_mutex_unlock(&tableMutex);
}

void *interfaceThread(void *arg) {
    char command[100];
    // Função para exibir a lista de participantes na tela
    displayParticipants();
        
    scanf("Commands: EXIT and WAKEUP:\n %s",command);
    if (command == "EXIT") {
        printf("exit");
    } else if (command == "WAKEUP") {
        // Enviar comando de WAKEUP (pacote WoL) para a estação especificada
    } else {
        printf("Comando inválido.");
    }
}



#endif
