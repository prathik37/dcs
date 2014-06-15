#include <string>
using namespace std;
// Client map structure sent and recieved
typedef struct sockaddr_in clientsock;
typedef struct
{
    string name;
    int DeadCount;
    int Extra;		
    bool IsAlive;
    int ACK_No;
    int SEQ_No;
    string CurrentleaderID;
    clientsock client;
}clientinfo;

//Chat Message structure sent and recieved

typedef struct
{
     string LeaderUID;
     string ClientUID;
     string GroupUID;
}UID;

typedef struct
{
    string Msg_Type;
    string client_name;
    int Seq_No;
    int Ack_No;
    UID Access;
    clientsock client;
    string user_message;
}user;

//Structure used to select(switch) function based on Msg_Type
typedef struct
{
     string Msg_Type;
     string Serialized_payload;
}recievedmsg;

