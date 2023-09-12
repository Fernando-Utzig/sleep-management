#ifndef C_INTERFACE
#define C_INTERFACE
#include "interface.h"


#define UNDEFINED_COMMAND -1
#define EXIT_COMMAND 1
#define WAKE_UP_COMMAND 2
#define SLEEP_COMMAND 3

pthread_mutex_t displayMutex;
FILE *interface_logfile ;
void setInterfaceLogFile(FILE *file)
{
    if(file != NULL)
        interface_logfile =file;
    else
    {
        interface_logfile=stderr;
        //fprintf(interface_logfile,"discovery got a null file, using std err instead \n");
    }
    //fprintf(interface_logfile,"interface_logfile set\n");
}

void removeEnterChar(char *string)
{
    if(string == NULL)
    {
        //fprintf(interface_logfile, "trying to remove \\n from NULL string");
        return;
    }
    int i;
    for(i=0;string[i]!='\n'&& string[i]!='\0';i++)
    {}
    if(string[i]=='\n')
        string[i]='\0';
    return;
}

void display()
{
    pthread_mutex_unlock(&displayMutex);
}

void *displayParticipantsTable(void *arg) {
    while(1)
    {
        pthread_mutex_lock(&displayMutex);
        system("clear"); // Comando para limpar a tela no Ubuntu
        printAllParticipants();
        printf("Commands: WAKEUP <Hostname>:\n");
    }
}


void *interfaceThreadParticipant(void *arg) {
    char command[256];
    char mac[256];
    command[0]='\0';
    char *fgetsreturn ="nao nulo";
    // Função para exibir a lista de participantes na tela
    //fprintf(interface_logfile,"Iniciating interfaceThreadParticipant\n");
    while(fgetsreturn!=NULL)
    {
        printf("Commands: EXIT:\n");
        //scanf("%s",command);
        fgetsreturn = fgets(command,256,stdin);
        if(fgetsreturn == NULL)
            raise(SIGINT);
        fflush(stdin);
        removeEnterChar(command);
        if (strcmp(command,"EXIT")==0) {
            raise(SIGINT);
        } else if (fgetsreturn!=NULL && strcmp(fgetsreturn,"\n")){
            printf("Comando inválido.\n");
        }
        if (strcmp(command,"MAN")==0) {
            raise(SIGUSR1);
        }
        fflush(stdin);
    }
}

int decodeAction(char* command, char*hostname)
{
    char wake[]="WAKEUP";
    char sleep[]="SLEEP";
    if(command == NULL)
    {
        //fprintf(interface_logfile,"command received is NULL");
        return UNDEFINED_COMMAND;
    }
    if(hostname == NULL)
    {
        //fprintf(interface_logfile,"hostname received is NULL");
        return UNDEFINED_COMMAND;
    }
    if(strcmp(command,"EXIT")==0)
        return EXIT_COMMAND;
    int i,result=0;
    for(i=0;i<6;i++)
    {
        result+=command[i]-wake[i];
    }
    if(result == 0)
    {
        //fprintf(interface_logfile,"command[7] = %c\n",command[7]);
        strcpy(hostname,&command[7]);
        //fprintf(interface_logfile,"wakeup command, hostname fould = %s\n",hostname);
        return WAKE_UP_COMMAND;
    }
    result=0;
    for(i=0;i<5;i++)
    {
        result+=command[i]-sleep[i];
    }
    if(result == 0)
    {
        strcpy(hostname,&command[6]);
        //fprintf(interface_logfile,"sleep command, mac fould = %s\n",hostname);
        return SLEEP_COMMAND;
    }

    return UNDEFINED_COMMAND;
}



void *interfaceThreadManager(void *arg) {
    char command[256];
    char hostname[256];
    command[0]='\0';
    Participant *participant;
    char *fgetsreturn ="nao nulo";
    int request_result;
    // Função para exibir a lista de participantes na tela
    //fprintf(interface_logfile,"Iniciating InterfaceThreadManager\n");
    while(fgetsreturn!=NULL)
    {
        
        //scanf("%s",command);
        fgetsreturn = fgets(command,256,stdin);
        if(fgetsreturn == NULL)//EOF
            raise(SIGINT);
        removeEnterChar(command);
        switch (decodeAction(command,hostname))
        {
        case WAKE_UP_COMMAND:
            removeEnterChar(hostname);
            participant = getParticipant(hostname);
            if(participant ==NULL)
            {
                printf("Did not found participant\n");
            }
            else
            {
                // Comando para enviar o pacote Wake-on-LAN
                char command[128];
                char prefix[] = "wakeonlan ";
                strcpy(command, prefix);
                strcat(command, participant->MAC);

                // Executando o comando
                system(command);
                if(request_result != 1)
                {
                    printf("Request failed\n");
                }
                else
                {
                    printf("Request Succefully\n");
                }
            }
            break;
        default:
            if(fgetsreturn!=NULL && fgetsreturn[0]!='\0' && fgetsreturn[0]!='\n'){
                printf("Comando inválido.\n");
                //fprintf(interface_logfile,"fgetsreturn = %s\n",fgetsreturn);
            }
            break;
        }
    }
}

#endif
