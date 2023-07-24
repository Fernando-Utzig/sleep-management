#ifndef C_INTERFACE
#define C_INTERFACE
#include "interface.h"


#define UNDEFINED_COMMAND -1
#define EXIT_COMMAND 1
#define WAKE_UP_COMMAND 2
#define SLEEP_COMMAND 3

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
    fprintf(stderr,"Iniciating interfaceThreadParticipant\n");
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
            printf("exit");
            fflush(stdout);
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
        fprintf(stderr,"command received is NULL");
        return UNDEFINED_COMMAND;
    }
    if(mac == NULL)
    {
        fprintf(stderr,"mac received is NULL");
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
        fprintf(stderr,"command[7] = %c\n",command[7]);
        strcpy(mac,&command[7]);
        fprintf(stderr,"wakeup command, mac fould = %s\n",mac);
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
        fprintf(stderr,"sleep command, mac fould = %s\n",mac);
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
    fprintf(stderr,"Iniciating InterfaceThreadManager\n");
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
            printf("exit not implemented yet");
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
            printf("Type in the MAC address\n "); // a especificação do trabalho pede que o comando seja wakeup <mac> não um segundo input, precisa criar uma funcao de decodificação
            fgets(mac,256,stdin);
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
                fprintf(stderr,"fgetsreturn = %s\n",fgetsreturn);
            }
            break;
        }
        if(getchar() !='\n') // to read a trailling \\n that make the fgets skip a input
            printf("erro lol kkkk");
    }
}

#endif
