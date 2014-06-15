#include <pthread.h>
#include <iostream>
#include <algorithm>
#include <map>
#include <mutex>

#include "thread_functions.h"
#include "ipfile.h"
#include "MessageSerialize.h"

using namespace std;


mutex client_send_map_mutex;
mutex client_receive_map_mutex;
mutex client_info_map_mutex;
mutex leader_send_receive_map_mutex;
mutex leader_client_info_map_mutex;

//client
bool client;
//key --- client message sequence number, value --- message payload
map<int,string> client_send_map;

//key --- leader message sequence number, value --- message payload
map<int,string> client_receive_map;

//to be used for printinf from the map
int client_last_seq_no_printed;

//global sequence number for entered messages in the UI
int client_seq_no;

int client_last_received_leader_seq;

//client map version number
int clientmap_version;

//client info map
map<string,clientinfo> client_info_map;

bool firstPacket;

//leader
bool leader;
//key --- leader message sequence number, value --- message payload
map<int,string> leader_send_receive_map;

//key --- leader message sequence number, value --- message payload
map<string,clientinfo> leader_client_info_map;

//to be used for printinf from the map
int leader_last_seq_no_printed;

//leader map version number
int leadermap_version;

//global sequence number for entered messages in the UI
int leader_seq_no;

int leader_last_deleted_message;

//Leader dead count
int Ldeadcount;

bool leader_elect_proc;

//client ID and client port
string clientID;
int clientport;
string clientIP;

//leader ID
string leaderID;
int leaderport;
string leaderIP;

//group ID - reverse of leaderID
string groupID;

string delimiter;

string chat_name;

bool leader_elected;

bool doneOnce;

bool switchMade;

int main(int argc, char *argv[])
{
    srand(time(0));
        pthread_t t_user_input, t_recv_msg, t_send_msg, t_print_msg, t_l_election;

        int error_value;

        client = false;
        leader = false;

        //multicast broadcast messafe sequencing
        firstPacket = true;
        //thread args
        long t0 = 0, t1 = 0, t2 = 0, t3 = 0, t4 = 0;

    leader_elected=false;
    leader_elect_proc=false;
    doneOnce=false;
    switchMade = false;
        int count = 0;


        //To split input
        char* tkn;


        delimiter="\r\n.\r\n";

        char name[1000];
        char lIP[100];

       // char dchat_ip[65535];


       // fgets(dchat_ip, 65535, stdin);

       // tkn = strtok(dchat_ip, " ,.-");


        if (argc > 3 || argc < 2)
        {
            cout<<"Invalid command line argument\n For starting a new chat : dchat <chat name>\n For joining an existing chat : dchat <chat name> <xxx.xxx.xxx.xxx:yyyyy>\n"<<flush;
        }


        if (argc == 2)
        {
            strcpy(name,argv[1]);
            chat_name = string(name);
            count = 2;
        }

        if(argc == 3)
        {

            strcpy(name,argv[1]);
            chat_name = string(name);
            strcpy(lIP,argv[2]);
            count = 3;

            if(strlen(lIP)<=10)
            {
                cout<<"Invalid IP & Port entered for joining.\nSample input --- xxx.xxx.xxx.xxx:yyyyy\n"<<flush;
                count=4;
            }

        }

    if(count==2)
    {
        leader = true;
        client = false;
        leaderIP = ipaddress();
        leaderport = rand() % 10000 + 10000;


        leaderID = leaderIP;
        leaderID.append(":");
        //to convert port no. to string to append
        stringstream dummy;
        dummy << leaderport;
        leaderID.append(dummy.str());

        cout<<chat_name<<" started a new chat, listening on "<< leaderID<<"\n"<<flush;
        cout<<"Succeeded, current users:\n"<<flush;
        cout<<chat_name<<" "<<leaderID<<" (Leader)\n"<<flush;
        cout<<"Waiting for others to join...\n"<<flush;




        //groupID is reversed leaderID
        groupID.assign(leaderID.rbegin(), leaderID.rend());

        //cout<<" leaderport "<<leaderport<< " leaderIP "<<leaderIP<<" leaderID "<<leaderID<<"\n";
        //cout<<" groupID "<<groupID<<"\n"<<flush;

    }
    else if(count==3)
    {
        client = true;
        leader = false;
        /*IP stored in the ascii format and whenever used is converted to the network format*/
        clientIP=ipaddress();
        //clientport=htons(29999);
        clientport = rand() % 10000 + 10000;


        /*storing the port in network format and is sent in the same format. When received at the leader used directly
          for printing there the converter ntohs should be used  */
        //clientport=htons(clientport);

        string splitlIP(lIP);
        string splitter = ":";
        leaderIP = splitlIP.substr(0, splitlIP.find(splitter));
        //cout<<"leader IP : "<<leaderIP<<endl;
        //cout<<"1. splitIP : "<<splitlIP<<endl;
        splitlIP.erase(0, leaderIP.length()+1);
        //cout<<"2. splitIP : "<<splitlIP<<endl;
        leaderport = atoi(splitlIP.c_str());
        //cout<<"leader port : "<<leaderport<<endl;
        clientID = clientIP;
        clientID.append(":");
        stringstream dummy;
        dummy << clientport;
        clientID.append(dummy.str());

        //to convert port no. to string to append
        cout<<chat_name<<" joining a new chat on "<<lIP<<", listening on "<<clientID<<"\n"<<flush;




        //For testing
        //cout<<" Clientport "<<clientport<< " ClientIP "<<clientIP<<" ClientID "<<clientID<<"\n"<<flush;



        //string assign_name(name);
        user Txmsg;
        Txmsg.Msg_Type="JOIN";
        Txmsg.client_name=chat_name;
        Txmsg.Seq_No=0;
        //Txmsg.Seq_No=client_seq_no;
        Txmsg.Ack_No=0;
        Txmsg.Access.LeaderUID=string("NULL");
        //cout<<"************************client id being sent : "<<clientID<<endl;
        Txmsg.Access.ClientUID=clientID;
        //cout<<"************************client id being sent : "<<Txmsg.Access.ClientUID<<endl;
        Txmsg.Access.GroupUID=string("NULL");
        Txmsg.client.sin_family=AF_INET;
        inet_aton(clientIP.c_str(),&Txmsg.client.sin_addr);
        Txmsg.client.sin_port=htons(clientport);
        Txmsg.user_message="I want to join the chat!!!!";
        string SerialisedString=SerializeChatMsg(Txmsg, delimiter);
        //Send SerialisedString to socket. Address of client in sockaddr struct(client) of Rxmsg

        //ACQUIRE client_send_map MUTEX
        client_send_map_mutex.lock();
            client_send_map.insert(pair<int,string>(client_seq_no,SerialisedString));
        client_send_map_mutex.unlock();
        //RELEASE client_send_map MUTEX

        //cout<<"Added join message with sequence number : "<<client_seq_no<<" to the client_send_map\n";

    }
    else
    {
        leader = false;
        client = false;
        //cout<<"Failed to initiate"<<flush;
    }



    //Check if it is dchat USER, create a new thread which waits for input.
    //set the client and leader values to be true accordingly

    //for testing
    //client=true;

    /*Thread Pool --- based on client/leader

    if(client)
    Thread to read the UI
    Thread to send messages from client_send_map
    Thread to receive messages(including timeout based service messages) from leader into client_receive_map
    Thread to print messages from the client_receive_map to the UI

    if(leader)
    Thread to read the UI
    Thread to send multicast messages from leader_send_receive_map to all clients
    Thread to receive messages from clients into the leader_send_receive_map
    Thread to send service messages from the leader
    Thread to print messages from the leader_send_receive_map to the UI

*/

    if(client || leader){
        error_value = pthread_create(&t_user_input, NULL, chat_input, (void *)t0);
        if (error_value)
        {
            //cout<<"ERROR; return code from pthread_create() is %d\n"<<error_value;
            pthread_exit(0);
        }

        //thread to send messages to the leader from the map
        error_value = pthread_create(&t_send_msg, NULL, send_msg, (void *)t1);
        if (error_value)
        {
            //cout<<"ERROR; return code from pthread_create() is %d\n"<<error_value;
            pthread_exit(0);
        }

        //thread to get the messages from the leader
        error_value = pthread_create(&t_recv_msg, NULL, get_msg, (void *)t2);
        if (error_value)
        {
            //cout<<"ERROR; return code from pthread_create() is %d\n"<<error_value;
            pthread_exit(0);
        }

        //thread to print messages from receive map to the UI
        error_value = pthread_create(&t_print_msg, NULL, print_msg, (void *)t3);
        if (error_value)
        {
            //cout<<"ERROR; return code from pthread_create() is %d\n"<<error_value;
            pthread_exit(0);
        }

       error_value = pthread_create(&t_l_election, NULL, leader_election, (void *)t4);
        if (error_value)
        {
            //cout<<"ERROR; return code from pthread_create() is %d\n"<<error_value;
            pthread_exit(0);
        }

        pthread_join(t_user_input,NULL);
        pthread_join(t_send_msg,NULL);
        pthread_join(t_recv_msg,NULL);
        pthread_join(t_print_msg,NULL);
       pthread_join(t_l_election,NULL);

        pthread_exit(NULL);
    }

}


