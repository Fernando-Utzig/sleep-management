#ifndef C_INTERFACE
#define C_INTERFACE
#include "interface.h"


void removeEnterChar(char *string)
{
    if(string == NULL)
    {
        fprintf(stderr, "trying to remove \\n from NULL string");
        return;
    }
    int i;
    for(i=0;string[i]!='\n'&& string[i]!='\0';i++)
    {}
    if(string[i]=='\n')
        string[i]='\0';
    return;
}

void *interfaceThreadParticipant(void *arg) {
    char command[256];
    char mac[256];
    command[0]='\0';
    char *fgetsreturn ="nao nulo";
    // Função para exibir a lista de participantes na tela
    fprintf(stderr,"Iniciating InterfaceThreadManager\n");
    printAllParticipants();
    while(fgetsreturn!=NULL)
    {
        printf("Commands: EXIT and WAKEUP:\n");
        //scanf("%s",command);
        fgetsreturn = fgets(command,256,stdin);
        if(fgetsreturn == NULL)
            raise(SIGINT);
        fflush(stdin);
        removeEnterChar(command);
        if (strcmp(command,"EXIT")==0) {
            printf("exit");
            fflush(stdout);
        } else if (strcmp(command,"WAKEUP")==0) {
            printf("\n");
            // Enviar comando de WAKEUP (pacote WoL) para a estação especificada
        } else if(fgetsreturn!=NULL){
            printf("Comando inválido.\n");
        }
        fflush(stdin);
    }
}



void *interfaceThreadManager(void *arg) {
    char command[256];
    char mac[256];
    command[0]='\0';
    char *fgetsreturn ="nao nulo";
    // Função para exibir a lista de participantes na tela
    fprintf(stderr,"Iniciating InterfaceThreadManager\n");
    printAllParticipants();
    while(fgetsreturn!=NULL)
    {
        printf("Commands: EXIT and WAKEUP:\n");
        //scanf("%s",command);
        fgetsreturn = fgets(command,256,stdin);
        if(fgetsreturn == NULL)
            raise(SIGINT);
        fflush(stdin);
        removeEnterChar(command);
        if (strcmp(command,"EXIT")==0) {
            printf("exit");
            fflush(stdout);
        } else if (strcmp(command,"WAKEUP")==0) {
            fflush(stdout);
            printf("Type in the MAC address\n ");
            scanf("%s",mac);
            printf("\n");
            // Enviar comando de WAKEUP (pacote WoL) para a estação especificada
        } else if(fgetsreturn!=NULL){
            printf("Comando inválido.\n");
        }
        fflush(stdin);
    }
    
}

#endif
