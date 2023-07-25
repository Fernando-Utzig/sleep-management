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
        fprintf(interface_logfile,"discovery got a null file, using std err instead \n");
    }
    fprintf(interface_logfile,"interface_logfile set\n");
}

void removeEnterChar(char *string)
{
    if(string == NULL)
    {
        fprintf(interface_logfile, "trying to remove \\n from NULL string");
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
        printAllParticipants();
    }
}


void *interfaceThreadParticipant(void *arg) {
    char command[256];
    char mac[256];
    command[0]='\0';
    char *fgetsreturn ="nao nulo";
    // Função para exibir a lista de participantes na tela
    fprintf(interface_logfile,"Iniciating interfaceThreadParticipant\n");
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
        } else if (fgetsreturn!=NULL){
            printf("Comando inválido.\n");
        }
        fflush(stdin);
    }
}

int decodeAction(char* command, char*mac)
{
    char wake[]="WAKEUP";
    char sleep[]="SLEEP";
    if(command == NULL)
    {
        fprintf(interface_logfile,"command received is NULL");
        return UNDEFINED_COMMAND;
    }
    if(mac == NULL)
    {
        fprintf(interface_logfile,"mac received is NULL");
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
        fprintf(interface_logfile,"command[7] = %c\n",command[7]);
        strcpy(mac,&command[7]);
        fprintf(interface_logfile,"wakeup command, mac fould = %s\n",mac);
        return WAKE_UP_COMMAND;
    }
    result=0;
    for(i=0;i<5;i++)
    {
        result+=command[i]-sleep[i];
    }
    if(result == 0)
    {
        strcpy(mac,&command[6]);
        fprintf(interface_logfile,"sleep command, mac fould = %s\n",mac);
        return SLEEP_COMMAND;
    }
        

    //if(strcmp(command,"WAKEUP")==0) // melhorar decodificação do wakeup e do sleep!
    //    return WAKE_UP_COMMAND;
    //if(strcmp(command,"SLEEP")==0)
    //    return SLEEP_COMMAND;
    return UNDEFINED_COMMAND;
}

void *printManagerThread(void *arg)
{
    printManager();
    sleep(3000);
}

void *interfaceThreadManager(void *arg) {
    char command[256];
    char mac[256];
    command[0]='\0';
    Participant *participant;
    char *fgetsreturn ="nao nulo";
    int request_result;
    // Função para exibir a lista de participantes na tela
    fprintf(interface_logfile,"Iniciating InterfaceThreadManager\n");
    printAllParticipants();
    while(fgetsreturn!=NULL)
    {
        printf("Commands: EXIT and WAKEUP:\n");
        //scanf("%s",command);
        fgetsreturn = fgets(command,256,stdin);
        if(fgetsreturn == NULL)//EOF
            raise(SIGINT);
        removeEnterChar(command);
        switch (decodeAction(command,mac))
        {
        case EXIT_COMMAND:
            raise(SIGINT);
            break;
        case WAKE_UP_COMMAND:
            removeEnterChar(mac);
            participant = getParticipant(mac);
            if(participant ==NULL)
            {
                printf("Did not found participant\n");
            }
            else
            {
                request_result = sleepOrWakupParticipant(participant,1);
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
        case SLEEP_COMMAND:
            removeEnterChar(mac);
            participant = getParticipant(mac);
            if(participant ==NULL)
            {
                printf("Did not found participant\n");
            }
            else
            {
                request_result = sleepOrWakupParticipant(participant,0);
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
            if(fgetsreturn!=NULL && fgetsreturn[0]!='\0'){
                printf("Comando inválido.\n");
                fprintf(interface_logfile,"fgetsreturn = %s\n",fgetsreturn);
            }
            break;
        }
        if(getchar() !='\n') // to read a trailling \\n that make the fgets skip a input
            printf("erro lol kkkk");
    }
}

#endif
