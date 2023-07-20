
#define Participant_Name_size 64

struct ParticipantData {
    char Hostname[Participant_Name_size];
    char MAC[Participant_Name_size];
    char ip_address[Participant_Name_size];
    int is_awaken;
}typedef Participant;

struct SubService_MetaData
{
    Participant* ParticipantTable;
    int ParticipantTable_size;
}typedef SMetaData;