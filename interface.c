#ifndef C_INTERFACE
#define C_INTERFACE
#include "interface.h"

void *interfaceThread(void *arg) {
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

#endif
