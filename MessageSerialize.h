#ifndef _MessageSerialize_H_
#define _MessageSerialize_H_

/////////Message Switch-Serialize/Deserialise Code//////////////
///////////////////////
/*EXTERN delimiters, GroupID
Remove input Message, and local maps
get MyUID,GroupID, Global_Send_Sequence_Number, Leader_Seq_Number variable
*///////////////////////
#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>
#include <sys/socket.h>
#include <netdb.h>
#include <map>
#include <iostream>
#include <string>
#include <sstream>
#include <string.h>
#include <vector>
#include "main.h"
//////////////////////////Structures//////////

using namespace std;

enum Message_Enum
{
    //Leader Recieves
    eJOIN,
    eJOIN_SUCCESS,
    eMULTICAST_REQUEST,
    eMULTICAST_BROADCAST_ACK,
    eLEADERACK,
    eLEAVE,

    //CLient Recieves
    eSERVICE,
    eJOIN_ACK_MEMBER,
    eJOIN_ACK_LEADER,
    eMULTICAST_REQUEST_ACK,
    eMULTICAST_BROADCAST,
    eLEADER,
    eLEAVE_SUCCESS,
    eNOTICE,

    INVALID
};

//////////////////////////////////////////////////////////


///////////////////////////Function Prototypes////////////
//Serialization
string SerializeMap( map<string,clientinfo> ClientTable,  string delimiter);
string SerializeChatMsg(user chatpayloadsent,  string delimeter);

//De-Serialization
recievedmsg DeserializeMsgType( string SerializedString,  string del);
map<string,clientinfo> DeserializeMap( string SerializedString,  string del);
user DeserializeChatMsg( string SerializedString_payload,  string del);

//Miscellaneous
vector< string> Tokenoize( string str,  string Delimiter);
int StringToInt( string str);

//Thread functions
void *get_msg(void *threadid);

//MessageSwitch
Message_Enum Get_e( string msg);
void ServiceJOIN(recievedmsg UnpackedMsg);
void ServiceJOIN_ACK_MEMBER();
void ServiceMULTICAST_REQUEST(recievedmsg UnpackedMsg);
void ServiceMULTICAST_REQUEST_ACK(recievedmsg UnpackedMsg);
void ServiceMULTICAST_BROADCAST_ACK(recievedmsg UnpackedMsg);
void ServiceLEAVE(recievedmsg UnpackedMsg);
void ServiceNOTICE(recievedmsg UnpackedMsg);

//Test Functions///////
void DisplayMap();
void DisplayClientMap();
void DisplayChatpayloadSendtruct(user chatpayloadtosend);
void DisplayClientSendMap();
//////////////////////

//////////////////////////////////////////////////////////


////////////////////Serialize/Deserialize////////////////
string SerializeMap(map<string,clientinfo> ClientTable, string delimiter)
{
    string SerializedString;
    ostringstream ostr;
    ostr << "SERVICE" << delimiter << ClientTable.size() << delimiter;

    for(map<string,clientinfo>::iterator it=ClientTable.begin();it != ClientTable.end(); it++)
    {
        ostr << it->first << delimiter << it->second.name << delimiter << it->second.DeadCount << delimiter << it->second.Extra << delimiter << it->second.IsAlive << delimiter << it->second.ACK_No << delimiter << it->second.SEQ_No << delimiter<< it->second.CurrentleaderID << delimiter << it->second.client.sin_family << delimiter << inet_ntoa(it->second.client.sin_addr) << delimiter <<it->second.client.sin_port;
        ostr << delimiter ;
    }
    SerializedString=ostr.str();
    return SerializedString;
}

string SerializeChatMsg(user chatpayloadsent, string delimeter)
{
    string Serialized_payload;
    ostringstream ostr;
    ostr << chatpayloadsent.Msg_Type << delimeter << chatpayloadsent.client_name << delimeter << chatpayloadsent.Seq_No << delimeter << chatpayloadsent.Ack_No << delimeter << chatpayloadsent.Access.LeaderUID << delimeter << chatpayloadsent.Access.ClientUID << delimeter << chatpayloadsent.Access.GroupUID << delimeter << chatpayloadsent.client.sin_family<< delimeter << inet_ntoa(chatpayloadsent.client.sin_addr)<< delimeter << chatpayloadsent.client.sin_port << delimeter <<chatpayloadsent.user_message << delimeter;
    Serialized_payload=ostr.str();
    return Serialized_payload;
}

recievedmsg DeserializeMsgType(string SerializedString, string del)
{
    recievedmsg payload;
    vector<string> strvector;
    strvector=Tokenoize(SerializedString,del);
    vector<string>::iterator itertype=strvector.begin();
    payload.Msg_Type=*itertype;
    payload.Serialized_payload=SerializedString;
    return payload;
}

map<string,clientinfo> DeserializeMap(string SerializedString, string del)
{
    map<string,clientinfo>RecieverTable;
    vector<string> strvector;
    string key;
    int map_size=0;

    strvector=Tokenoize(SerializedString,del);
    /*for(vector<string>::iterator iter=strvector.begin(); iter != strvector.end(); iter++)
    {
        cout << *iter;
    }
    cout<<"\n";*/

    vector<string>::iterator iterstring=strvector.begin();
    if((*iterstring).compare("SERVICE") == 0)
    {
        map_size=StringToInt(*(++iterstring));
        for(size_t i=0; i < map_size; i++)
        {
            key=*(++iterstring);
            RecieverTable[key].name=*(++iterstring);
            RecieverTable[key].DeadCount=StringToInt(*(++iterstring));
            RecieverTable[key].Extra=StringToInt(*(++iterstring));
            RecieverTable[key].IsAlive=StringToInt(*(++iterstring));
            RecieverTable[key].ACK_No=StringToInt(*(++iterstring));
            RecieverTable[key].SEQ_No=StringToInt(*(++iterstring));
            RecieverTable[key].CurrentleaderID=*(++iterstring);
            RecieverTable[key].client.sin_family=StringToInt(*(++iterstring));
            inet_aton((*(++iterstring)).c_str(),&RecieverTable[key].client.sin_addr);
            RecieverTable[key].client.sin_port=StringToInt(*(++iterstring));
        }
    }
    else
        //cout<<"SERVICE ERROR";
    return RecieverTable;
}

user DeserializeChatMsg(string SerializedString_payload, string del)
{
    user chatpayloadrecv;
    vector<string> strvector;
    strvector=Tokenoize(SerializedString_payload, del);
    vector<string>::iterator iterchat=strvector.begin();
    chatpayloadrecv.Msg_Type=*(iterchat);
    chatpayloadrecv.client_name=*(++iterchat);
    chatpayloadrecv.Seq_No=StringToInt(*(++iterchat));
    chatpayloadrecv.Ack_No=StringToInt(*(++iterchat));
    chatpayloadrecv.Access.LeaderUID=*(++iterchat);
    chatpayloadrecv.Access.ClientUID=*(++iterchat);
    chatpayloadrecv.Access.GroupUID=*(++iterchat);
    chatpayloadrecv.client.sin_family=StringToInt(*(++iterchat));
    inet_aton((*(++iterchat)).c_str(),&chatpayloadrecv.client.sin_addr);
    chatpayloadrecv.client.sin_port=StringToInt(*(++iterchat));
    chatpayloadrecv.user_message=*(++iterchat);
    return chatpayloadrecv;
}

vector<string> Tokenoize(string str, string Delimiter)
{
    vector<string> strvect;
    size_t pos=0, last_pos=0;
    while(last_pos != string::npos)
    {
        last_pos=str.find(Delimiter,last_pos+1);
        if(last_pos != string::npos)
            strvect.push_back(str.substr(pos,(last_pos-pos)));
        else
            strvect.push_back(str.substr(pos,(str.size()-pos)));
        pos=Delimiter.size()+last_pos;
    }
    return strvect;
}

int StringToInt(string str)
{
    int num=0;
    stringstream convert(str);
    convert >> num;
    return num;
}
/////////////////////////////////////////////////////////

Message_Enum Get_e( string msg)
{
    if(msg=="JOIN") return eJOIN;
    else if(msg=="JOIN_SUCCESS") return eJOIN_SUCCESS;
    else if(msg=="MULTICAST_REQUEST") return eMULTICAST_REQUEST;
    else if(msg=="MULTICAST_BROADCAST_ACK") return eMULTICAST_BROADCAST_ACK;
    else if(msg=="LEADERACK") return eLEADERACK;
    else if(msg=="LEAVE") return eLEAVE;

    else if(msg=="SERVICE") return eSERVICE;
    else if(msg=="JOIN_ACK_MEMBER") return eJOIN_ACK_MEMBER;
    else if(msg=="JOIN_ACK_LEADER") return eJOIN_ACK_LEADER;
    else if(msg=="MULTICAST_REQUEST_ACK") return eMULTICAST_REQUEST_ACK;
    else if(msg=="MULTICAST_BROADCAST") return eMULTICAST_BROADCAST;
    else if(msg=="LEADER") return eLEADER;
    else if(msg=="LEAVE_SUCCESS") return eLEAVE_SUCCESS;
    else if(msg=="NOTICE") return eNOTICE;

    else return INVALID;
}

///////For Testing
void DisplayMap();
void DisplayClientMap();
void DisplayChatpayloadSendtruct(user chatpayloadtosend);
void DisplayClientSendMap();
///////


void ServiceLEADER(recievedmsg UnpackedMsg)
{
    //cout<<"Entered Service Leader"<<"\n";

    ////////////////////////////
    /*
     *Check if client exist.
     *if not return;
     *He may loose connection to leader and send leader election, when actually leader exists for all
     */
    ////////////////////////////
    leader_elect_proc=false;
    if(clientport !=0 )
    {
        //cout<<"Updated Leader Variables"<<"\n";
        leader_elected = true;
        user Rxmsg;
        Rxmsg=DeserializeChatMsg(UnpackedMsg.Serialized_payload, delimiter);
        //cout<<"Recieved Leader Serialized Message:"<<UnpackedMsg.Serialized_payload<<"\n";

        //Set itself as the leader
        leader=true;
        client=false;
        leaderID=clientID;
        clientID.clear();
        leaderIP=clientIP;
        clientIP.clear();
        leaderport=clientport;
        clientport=0;


        //ACQUIRE client_info_map mutex
        client_info_map_mutex.lock();

        //ACQUIRE leader_client_infor_map mutex
        leader_client_info_map_mutex.lock();

        //ACQUIRE leader_send_recieve_map mutex
        leader_send_receive_map_mutex.lock();

        //map<string,clientinfo>::iterator iterfind = client_info_map.begin();
        //leader_seq_no=iterfind->second.Extra;
        //cout<<"leader sequence number received : "<<leader_seq_no<<endl;
        leader_seq_no=0;
        leader_last_seq_no_printed=0;
        leader_last_deleted_message=0;
        leader_send_receive_map.clear();

        //Remove my entry from map
        client_info_map.erase(leaderID);
        leader_client_info_map=client_info_map;
        client_info_map.clear();

        for(map<string,clientinfo>::iterator iterclear =leader_client_info_map.begin(); iterclear != leader_client_info_map.end(); iterclear++)
        {
            iterclear->second.ACK_No=0;
            iterclear->second.CurrentleaderID=leaderID;
        }

        //RELEASE leader_send_recieve_map mutex
        leader_send_receive_map_mutex.unlock();

        //RELEASE leader_client_infor_map mutex
        leader_client_info_map_mutex.unlock();

        //RELEASE client_info_map mutex
        client_info_map_mutex.unlock();
        cout<<"I'm the new leader\n"<<flush;
        cout<<"LeaderIP:"<<leaderIP<<"\n"<<flush;
        cout<<"LeaderID:"<<leaderID<<"\n\n"<<flush;

        /*cout<<"LeaderID:"<<leaderID<<"\n";
            cout<<"LeaderIP:"<<leaderIP<<"\n";
            cout<<"PortID:"<<leaderport<<"\n";
            cout<<"Expect sending of Service message"<<"\n";

            cout<<"\n\n\n";*/
    }
}

void ServiceJOIN(recievedmsg UnpackedMsg)
{

    if(leader)
    {
        user Rxmsg, Txmsg, TxNotice;
        clientinfo RxClientData;
        string NoticeMessage, SerialisedString;
        ostringstream ostr, ostrnotice;
        //cout<<"REcieved Msg_Type in JOIN function:"<<UnpackedMsg.Msg_Type << "\n";
        //cout<<"SerializesPayoad"<<UnpackedMsg.Serialized_payload<<"\n";
        Rxmsg=DeserializeChatMsg(UnpackedMsg.Serialized_payload, delimiter);
        //cout<<"Delimiter used"<<delimiter<<"\n";
        //cout<<"Recieved Client Name:"<<Rxmsg.client_name<<"\n";
        // Add client structure to map
        RxClientData.name=Rxmsg.client_name;
        RxClientData.DeadCount=0;
        RxClientData.Extra=0;
        RxClientData.IsAlive=true;
        RxClientData.ACK_No=0;                                                                                              //should not set the ack number, would be set by the receive thread
        RxClientData.SEQ_No=Rxmsg.Seq_No;
        RxClientData.CurrentleaderID=leaderID;
        RxClientData.client.sin_family=Rxmsg.client.sin_family;
        RxClientData.client.sin_addr=Rxmsg.client.sin_addr;
        RxClientData.client.sin_port=Rxmsg.client.sin_port;
        /*cout<<"IP address returned by Deserializer:"<<inet_ntoa(Rxmsg.client.sin_addr)<<"\n";
    cout<<"IP address put in map:"<<inet_ntoa(RxClientData.client.sin_addr)<<"\n";
    cout<<"Port returned by Deserializer:"<< Rxmsg.client.sin_port<<"\n";
    cout<<"Port Sent put in map:"<< RxClientData.client.sin_port<<"\n";*/

        //ACQUIRE leader_client_info_map MUTEX
        leader_client_info_map_mutex.lock();

        //ACQUIRE leader_send_recieve_map MUTEX
        leader_send_receive_map_mutex.lock();


        ////////////////Sending Notice
        //cout<<"Test1"<<"\n";

        ostrnotice << "NOTICE " << Rxmsg.client_name << " joined on " << Rxmsg.Access.ClientUID << "\n";

        cout<<ostrnotice.str()<<flush;

        if(!leader_client_info_map.empty())
        {
            int socket_notice=socket(AF_INET,SOCK_DGRAM,0), r;

            TxNotice.Msg_Type="NOTICE";
            TxNotice.client_name=chat_name;
            TxNotice.Seq_No=0;
            TxNotice.Ack_No=0;
            TxNotice.Access.LeaderUID=leaderID;
            //cout<<"------------------------leader id sent : "<<leaderID<<endl;
            TxNotice.Access.ClientUID="ClientUID";
            //cout<<"------------------------group id sent : "<<groupID<<endl;
            TxNotice.Access.GroupUID=groupID;
            //cout<<"------------------------group id sent : "<<Txmsg.Access.GroupUID<<endl;
            TxNotice.client.sin_family=AF_INET;

            inet_aton(leaderIP.c_str(),&TxNotice.client.sin_addr);
            TxNotice.client.sin_port=leaderport;
            TxNotice.user_message=ostrnotice.str();
            NoticeMessage=SerializeChatMsg(TxNotice, delimiter);

            for (map<string,clientinfo>::iterator notice_iterator=leader_client_info_map.begin(); notice_iterator!=leader_client_info_map.end(); ++notice_iterator)
            {
                char buffer[65535];
                int length = NoticeMessage.copy(buffer,NoticeMessage.length(),0);
                buffer[length]='\0';
                //cout<<"leader map size : "<<leader_send_receive_map.size()<<endl;
                int status = sendto(socket_notice,buffer,strlen(buffer),0,(struct sockaddr *)&notice_iterator->second.client,sizeof(notice_iterator->second.client));
                //cout<<"In client_send_message thread function \n Sequence Number sent : "<<leader_send_map_iterator->first<<" and message sent size : "<<status<<endl;

            }
        }
        //////////////////////////////



        pair<map<string,clientinfo>::iterator,bool> checknewclient;
        //cout<<"$$$$$$$$$$$$$$$$$$$$$$$$ client id received at the leader : "<<Rxmsg.Access.ClientUID<<"\n";
        checknewclient=leader_client_info_map.insert(pair<string,clientinfo>(Rxmsg.Access.ClientUID,RxClientData));



        if(checknewclient.second == false)
        {
            //cout<<"Client already present"<<"\n";
            //return;
        }


        //Send JOIN_ACK_LEADER to client
        // Create return message of current users.


        ostr<< chat_name << "\t" << leaderID << " (leader)" << "\n";
        for(map<string,clientinfo>::iterator itermap= leader_client_info_map.begin(); itermap != leader_client_info_map.end(); ++itermap)
        {
            ostr<<itermap->second.name << "\t" << itermap->first << "\n";
        }

        map<string,clientinfo>::iterator it=leader_client_info_map.find(Rxmsg.Access.ClientUID);                        //Access new clients info
        if(it!=leader_client_info_map.end())
        {
            if(it->second.ACK_No<=Rxmsg.Seq_No)
            {
                Txmsg.Msg_Type="JOIN_ACK_LEADER";
                Txmsg.client_name=Rxmsg.client_name;
                Txmsg.Seq_No=0;
                Txmsg.Ack_No=Rxmsg.Seq_No;
                Txmsg.Access.LeaderUID=leaderID;
                //cout<<"------------------------leader id sent : "<<leaderID<<endl;
                Txmsg.Access.ClientUID=Rxmsg.Access.ClientUID;
                //cout<<"------------------------group id sent : "<<groupID<<endl;
                Txmsg.Access.GroupUID=groupID;
                //cout<<"------------------------group id sent : "<<Txmsg.Access.GroupUID<<endl;
                Txmsg.client.sin_family=AF_INET;

                inet_aton(leaderIP.c_str(),&Txmsg.client.sin_addr);
                Txmsg.client.sin_port=leaderport;
                Txmsg.user_message=ostr.str();
                SerialisedString=SerializeChatMsg(Txmsg, delimiter);

                //Send SerialisedString to socket. Address of client in sockaddr struct(client) of Rxmsg


                int socket_id=socket(AF_INET,SOCK_DGRAM,0), r;

                char buffer[65535];
                int length = SerialisedString.copy(buffer,SerialisedString.length(),0);
                buffer[length]='\0';
                /*cout<<"buffer : \n\n"<<buffer<<endl;
                cout<<"string : \n\n"<<send_map_iterator->second<<endl;*/
                //int status = sendto(socket_id,buffer,sizeof(buffer),0,(struct sockaddr *)&MaxClient->second.client,sizeof(MaxClient->second.client));

                r = sendto(socket_id,buffer,strlen(buffer),0,(struct sockaddr *)&it->second.client,sizeof(it->second.client));
                //cout<<"Sent JOIN_ACK_LEADER message:"<<SerialisedString.c_str()<<"\n";
                //cout<<"sizeof message:"<<r<<"\n";



                close(socket_id);
                //    cout<<"Unsuccessfully sent"<<"\n";
                /*
                cout<<"sizeof message:"<<r<<"\n";
                cout<<"Family sent:"<<it->second.client.sin_family<<"\n";
                    cout<<"IP address sent:"<<inet_ntoa(it->second.client.sin_addr)<<"\n";
                    cout<<"Port Sent:"<<it->second.client.sin_port<<"\n";
                    cout<<"Struct Sequence Number"<<it->second.SEQ_No<<"\n";
                    cout<<"Sent ACK Message:"<< SerialisedString.c_str() <<"\n";
                    cout<<"Sent to"<< it->first <<"\n";*/
            }
        }
        leader_send_receive_map_mutex.unlock();
        //RELEASE leader_send_recieve_map MUTEX

        leader_client_info_map_mutex.unlock();
        //RELEASE leader_client_info_map MUTEX
    }

    if(client)
    {

        if(!leader_elect_proc){

        user Rxmsg, Txmsg;
        string SerialisedString;

        Rxmsg=DeserializeChatMsg(UnpackedMsg.Serialized_payload, delimiter);

        //cout<<"Redirecting from client to leader"<<endl;

        Txmsg.Msg_Type="JOIN_ACK_MEMBER";
        Txmsg.client_name=chat_name;
        Txmsg.Seq_No=0;
        Txmsg.Ack_No=Rxmsg.Seq_No;
        Txmsg.Access.LeaderUID=leaderID;
        //cout<<"------------------------leader id sent : "<<leaderID<<endl;
        Txmsg.Access.ClientUID=Rxmsg.Access.ClientUID;
        //cout<<"------------------------group id sent : "<<groupID<<endl;
        Txmsg.Access.GroupUID="";
        //cout<<"------------------------group id sent : "<<Txmsg.Access.GroupUID<<endl;
        Txmsg.client.sin_family=AF_INET;

        inet_aton(leaderIP.c_str(),&Txmsg.client.sin_addr);
        //cout<<"This is the leaderIP that I have"<<leaderIP<<endl;
        Txmsg.client.sin_port=htons(leaderport);
        // cout<<"This is the leaderport that I have"<<ntohs(Txmsg.client.sin_port)<<endl;
        // cout<<"This is the leaderIP that I have"<<inet_ntoa(Txmsg.client.sin_addr)<<endl;

        Txmsg.user_message="Here you go!!!";
        SerialisedString=SerializeChatMsg(Txmsg, delimiter);

        //cout<<SerialisedString;

        //Send SerialisedString to socket. Address of client in sockaddr struct(client) of Rxmsg


        int socket_id=socket(AF_INET,SOCK_DGRAM,0), r;

        char buffer[65535];
        int length = SerialisedString.copy(buffer,SerialisedString.length(),0);
        buffer[length]='\0';
        /*cout<<"buffer : \n\n"<<buffer<<endl;
        cout<<"string : \n\n"<<send_map_iterator->second<<endl;*/
        //int status = sendto(socket_id,buffer,sizeof(buffer),0,(struct sockaddr *)&MaxClient->second.client,sizeof(MaxClient->second.client));

        r = sendto(socket_id,buffer,strlen(buffer),0,(struct sockaddr *)&Rxmsg.client,sizeof(Rxmsg.client));
        //cout<<"Sent JOIN_ACK_LEADER message:"<<SerialisedString.c_str()<<"\n";
        //cout<<"sizeof message:"<<r<<"\n";

        close(socket_id);

        }

    }


}

void ServiceJOIN_ACK_MEMBER(recievedmsg UnpackedMsg)
{
    user Rxmsg, Txmsg;
    struct sockaddr_in SendToLeader;
    Rxmsg=DeserializeChatMsg(UnpackedMsg.Serialized_payload, delimiter);

    client_send_map_mutex.lock();

    map<int,string>::iterator send_map_iterator = client_send_map.find(Rxmsg.Ack_No);
    // cout<<"Contents of map before deletion"<<"\t"<<"Key"<<"\t"<<"Value"<<"\n"<<send_map_iterator->first<<"\t"<<send_map_iterator->second<<"\n";

    if(!client_send_map.empty()){
        client_send_map.erase(send_map_iterator);
        // cout<<"entry deleted from client send map\n";
    }

    client_send_map_mutex.unlock();


    string splitlIP(Rxmsg.Access.LeaderUID.c_str());
    string splitter = ":";
    leaderIP = splitlIP.substr(0, splitlIP.find(splitter));
    //cout<<"leader IP : "<<leaderIP<<endl;
    //cout<<"1. splitIP : "<<splitlIP<<endl;
    splitlIP.erase(0, leaderIP.length()+1);
    //cout<<"2. splitIP : "<<splitlIP<<endl;
    leaderport = atoi(splitlIP.c_str());
    //usleep(100000);


    Txmsg.Msg_Type="JOIN";
    Txmsg.client_name=chat_name;
    Txmsg.Seq_No=0;
    Txmsg.Ack_No=0;
    Txmsg.Access.LeaderUID="s";
    //cout<<"************************client id being sent : "<<clientID<<endl;
    Txmsg.Access.ClientUID=clientID;
    //cout<<"************************client id being sent : "<<Txmsg.Access.ClientUID<<endl;
    Txmsg.Access.GroupUID="sa";
    Txmsg.client.sin_family=AF_INET;
    inet_aton(clientIP.c_str(),&Txmsg.client.sin_addr);
    Txmsg.client.sin_port=htons(clientport);
    Txmsg.user_message="Got info from client I want to join the chat!!!!";
    string SerialisedString=SerializeChatMsg(Txmsg, delimiter);
    //Send SerialisedString to socket. Address of client in sockaddr struct(client) of Rxmsg

    SendToLeader.sin_family=AF_INET;
    inet_aton(leaderIP.c_str(),&SendToLeader.sin_addr);
    SendToLeader.sin_port=htons(leaderport);
    int socket_id=socket(AF_INET,SOCK_DGRAM,0), r;

    char buffer[65535];
    int length = SerialisedString.copy(buffer,SerialisedString.length(),0);
    buffer[length]='\0';
    r = sendto(socket_id,buffer,strlen(buffer),0,(struct sockaddr *)&SendToLeader,sizeof(SendToLeader));


}


/*service join ack received from the leader*/
void ServiceJOIN_ACK_LEADER(recievedmsg UnpackedMsg)
{
    user Rxmsg;
    Rxmsg=DeserializeChatMsg(UnpackedMsg.Serialized_payload, delimiter);
    //cout<<"Recieved Serialized JOIN_ACK:"<<UnpackedMsg.Serialized_payload<<"\n";
    //cout<<"$$$$$$$$$$$$$$$$$$$$$$$$$$$group id received by the client : "<<Rxmsg.Access.GroupUID<<endl;
    groupID=Rxmsg.Access.GroupUID;
    leaderID=Rxmsg.Access.LeaderUID;
    //cout<<"ServiceJOIN_ACK_LEADER+++++++++++++++++++++++group id stored  : "<<groupID<<endl;
    //cout<<"ServiceJOIN_ACK_LEADER+++++++++++++++++++++++leaderID stored  : "<<leaderID<<endl;
    //ACQUIRE client_send_map MUTEX

    cout<<Rxmsg.user_message<<"\n";

    //ACQUIRE client_send_map mutex
    client_send_map_mutex.lock();
    map<int,string>::iterator send_map_iterator = client_send_map.find(Rxmsg.Ack_No);
    if(send_map_iterator!=client_send_map.end()){
        client_send_map.erase(send_map_iterator);
        //cout<<"entry deleted from client send map\n";
    }
    //RELEASE client_send_map mutex
    client_send_map_mutex.unlock();

}

void ServiceMULTICAST_REQUEST(recievedmsg UnpackedMsg)
{
    user Rxmsg, Txmsg;
    clientinfo RxClientinfo;
    string SerialisedString;

    Rxmsg=DeserializeChatMsg(UnpackedMsg.Serialized_payload, delimiter);

    //++leader_seq_no;                                                                                                        //Increment Send Broadcast Sequence Numeber

    // Update Clients Sequence Number
    //ACQUIRE leader_client_info_map MUTEX
    /*for(map<string,clientinfo>::iterator itermap=leader_client_info_map.begin(); itermap != leader_client_info_map.end(); itermap++)
    {
        RxClientinfo=itermap->second;
        RxClientinfo.SEQ_No=leader_seq_no;
        leader_client_info_map[itermap->first]=RxClientinfo;
    }*/
    //DisplayClientMap();
    //RELEASE leader_client_info_map MUTEX


    //Send MULTICAST_REQUEST_ACK to Rx

    //ACQUIRE leader_client_info_map MUTEX
    leader_client_info_map_mutex.lock();

    //ACQUIRE leader_send_receive_map MUTEX
    leader_send_receive_map_mutex.lock();

    map<string,clientinfo>::iterator it=leader_client_info_map.find(Rxmsg.Access.ClientUID);                             //Access new clients info
    if(it!=leader_client_info_map.end())
    {
        /*We send back the acknowledgement because the acknowledgement might have been lost or
                it is a new message which anyways need to be acknowledged*/


        //commented to solve the packet drop issue
        /*Txmsg.Msg_Type="MULTICAST_REQUEST_ACK";
            Txmsg.client_name=chat_name;
            Txmsg.Seq_No=0;
            Txmsg.Ack_No=Rxmsg.Seq_No;
            //cout<<endl<<"ServiceMULTICAST_REQUEST:: Sending MULTICAST_REQUEST_ACK with groupId : "<<groupID<<endl<<endl;
            Txmsg.Access.LeaderUID=leaderID;
            Txmsg.Access.ClientUID=Rxmsg.Access.ClientUID;
            Txmsg.Access.GroupUID=groupID;
            Txmsg.client.sin_family=AF_INET;
            inet_aton(leaderIP.c_str(),&Txmsg.client.sin_addr);
            Txmsg.client.sin_port=leaderport;
            Txmsg.user_message=Rxmsg.user_message;
            SerialisedString=SerializeChatMsg(Txmsg, delimiter);

            //Send SerialisedString to socket. Address of client in sockaddr struct(client) of Rxmsg
            int socket_id=socket(AF_INET,SOCK_DGRAM,0);

            char buffer[65535];
            int length = SerialisedString.copy(buffer,SerialisedString.length(),0);
            buffer[length]='\0';
            //int status = sendto(socket_id,buffer,sizeof(buffer),0,(struct sockaddr *)&MaxClient->second.client,sizeof(MaxClient->second.client));


            sendto(socket_id,buffer,strlen(buffer),0,(struct sockaddr *)&(it->second.client),sizeof(it->second.client));
            close(socket_id);*/
        /*we add the requested message to the leader map only if the incoming seq.no. is one plus the previously received value
             for maintaining the retransmission system and accepting packets in order
            */
        if(leader_elect_proc && !doneOnce){

            Txmsg.Msg_Type="MULTICAST_REQUEST_ACK";
            Txmsg.client_name=chat_name;
            Txmsg.Seq_No=0;
            Txmsg.Ack_No=Rxmsg.Seq_No;
            //cout<<endl<<"ServiceMULTICAST_REQUEST:: Sending MULTICAST_REQUEST_ACK with groupId : "<<groupID<<endl<<endl;
            Txmsg.Access.LeaderUID=leaderID;
            Txmsg.Access.ClientUID=Rxmsg.Access.ClientUID;
            Txmsg.Access.GroupUID=groupID;
            Txmsg.client.sin_family=AF_INET;
            inet_aton(leaderIP.c_str(),&Txmsg.client.sin_addr);
            Txmsg.client.sin_port=leaderport;
            Txmsg.user_message=Rxmsg.user_message;
            SerialisedString=SerializeChatMsg(Txmsg, delimiter);

            //Send SerialisedString to socket. Address of client in sockaddr struct(client) of Rxmsg
            int socket_id=socket(AF_INET,SOCK_DGRAM,0);

            char buffer[65535];
            int length = SerialisedString.copy(buffer,SerialisedString.length(),0);
            buffer[length]='\0';
            //int status = sendto(socket_id,buffer,sizeof(buffer),0,(struct sockaddr *)&MaxClient->second.client,sizeof(MaxClient->second.client));


            sendto(socket_id,buffer,strlen(buffer),0,(struct sockaddr *)&(it->second.client),sizeof(it->second.client));
            close(socket_id);

            pair<map<int,string>::iterator,bool> checknewclient;

            //Broadcast Packet
            user broadcastPacket;
            string broadcastPacketString;
            broadcastPacket.Msg_Type="MULTICAST_BROADCAST";
            broadcastPacket.client_name=chat_name;
            broadcastPacket.Seq_No=++leader_seq_no;
            broadcastPacket.Ack_No=0;
            broadcastPacket.Access.LeaderUID=leaderID;
            broadcastPacket.Access.ClientUID=Rxmsg.Access.ClientUID;
            broadcastPacket.Access.GroupUID=groupID;
            broadcastPacket.client.sin_family=AF_INET;
            inet_aton(leaderIP.c_str(),&broadcastPacket.client.sin_addr);
            broadcastPacket.client.sin_port=leaderport;
            broadcastPacket.user_message=Rxmsg.user_message;
            broadcastPacketString=SerializeChatMsg(broadcastPacket, delimiter);

            checknewclient=leader_send_receive_map.insert(pair<int,string>(leader_seq_no,broadcastPacketString));
            if(checknewclient.second == false)
            {
                //cout<<"Error: leader_seq_no not updated"<<"\n";
                return;
            }

            //update clientInfo to the latest value of seq_no seen by the leader from this particular client
            it->second.SEQ_No = Rxmsg.Seq_No;
            doneOnce=true;
        }
        else if(Rxmsg.Seq_No == it->second.SEQ_No+1){
            // Send message to leader_send_receive_map

            Txmsg.Msg_Type="MULTICAST_REQUEST_ACK";
            Txmsg.client_name=chat_name;
            Txmsg.Seq_No=0;
            Txmsg.Ack_No=Rxmsg.Seq_No;
            //cout<<endl<<"ServiceMULTICAST_REQUEST:: Sending MULTICAST_REQUEST_ACK with groupId : "<<groupID<<endl<<endl;
            Txmsg.Access.LeaderUID=leaderID;
            Txmsg.Access.ClientUID=Rxmsg.Access.ClientUID;
            Txmsg.Access.GroupUID=groupID;
            Txmsg.client.sin_family=AF_INET;
            inet_aton(leaderIP.c_str(),&Txmsg.client.sin_addr);
            Txmsg.client.sin_port=leaderport;
            Txmsg.user_message=Rxmsg.user_message;
            SerialisedString=SerializeChatMsg(Txmsg, delimiter);

            //Send SerialisedString to socket. Address of client in sockaddr struct(client) of Rxmsg
            int socket_id=socket(AF_INET,SOCK_DGRAM,0);

            char buffer[65535];
            int length = SerialisedString.copy(buffer,SerialisedString.length(),0);
            buffer[length]='\0';
            //int status = sendto(socket_id,buffer,sizeof(buffer),0,(struct sockaddr *)&MaxClient->second.client,sizeof(MaxClient->second.client));


            sendto(socket_id,buffer,strlen(buffer),0,(struct sockaddr *)&(it->second.client),sizeof(it->second.client));
            close(socket_id);

            pair<map<int,string>::iterator,bool> checknewclient;

            //Broadcast Packet
            user broadcastPacket;
            string broadcastPacketString;
            broadcastPacket.Msg_Type="MULTICAST_BROADCAST";
            broadcastPacket.client_name=chat_name;
            broadcastPacket.Seq_No=++leader_seq_no;
            broadcastPacket.Ack_No=0;
            broadcastPacket.Access.LeaderUID=leaderID;
            broadcastPacket.Access.ClientUID=Rxmsg.Access.ClientUID;
            broadcastPacket.Access.GroupUID=groupID;
            broadcastPacket.client.sin_family=AF_INET;
            inet_aton(leaderIP.c_str(),&broadcastPacket.client.sin_addr);
            broadcastPacket.client.sin_port=leaderport;
            broadcastPacket.user_message=Rxmsg.user_message;
            broadcastPacketString=SerializeChatMsg(broadcastPacket, delimiter);

            checknewclient=leader_send_receive_map.insert(pair<int,string>(leader_seq_no,broadcastPacketString));
            if(checknewclient.second == false)
            {
                //cout<<"Error: leader_seq_no not updated"<<"\n";
                return;
            }

            //update clientInfo to the latest value of seq_no seen by the leader from this particular client
            it->second.SEQ_No = Rxmsg.Seq_No;

            //DisplayClientSendMap();


            //RELEASE leader_send_receive_map MUTEX
        }else if(Rxmsg.Seq_No == it->second.SEQ_No || Rxmsg.Seq_No+1 == it->second.SEQ_No){

            //retransmit acks for last and second last retransmitted packet

            Txmsg.Msg_Type="MULTICAST_REQUEST_ACK";
            Txmsg.client_name=chat_name;
            Txmsg.Seq_No=0;
            Txmsg.Ack_No=Rxmsg.Seq_No;
            //cout<<endl<<"ServiceMULTICAST_REQUEST:: Sending MULTICAST_REQUEST_ACK with groupId : "<<groupID<<endl<<endl;
            Txmsg.Access.LeaderUID=leaderID;
            Txmsg.Access.ClientUID=Rxmsg.Access.ClientUID;
            Txmsg.Access.GroupUID=groupID;
            Txmsg.client.sin_family=AF_INET;
            inet_aton(leaderIP.c_str(),&Txmsg.client.sin_addr);
            Txmsg.client.sin_port=leaderport;
            Txmsg.user_message=Rxmsg.user_message;
            SerialisedString=SerializeChatMsg(Txmsg, delimiter);

            //Send SerialisedString to socket. Address of client in sockaddr struct(client) of Rxmsg
            int socket_id=socket(AF_INET,SOCK_DGRAM,0);

            char buffer[65535];
            int length = SerialisedString.copy(buffer,SerialisedString.length(),0);
            buffer[length]='\0';
            //int status = sendto(socket_id,buffer,sizeof(buffer),0,(struct sockaddr *)&MaxClient->second.client,sizeof(MaxClient->second.client));


            sendto(socket_id,buffer,strlen(buffer),0,(struct sockaddr *)&(it->second.client),sizeof(it->second.client));
            close(socket_id);
            //cout<<"given an ack back ... \nMessage Received Sequence number : "<<Rxmsg.Seq_No<<" Last msg scene from this client is : "<<it->second.SEQ_No<<endl;
        }else{
            //cout<<"Message Received Sequence number : "<<Rxmsg.Seq_No<<" Last msg scene from this client is : "<<it->second.SEQ_No<<endl;
        }
    }else{
        //cout<<"Servicing multicast request : client not found in the map, clientid value received is : "<<Rxmsg.Access.ClientUID;
        /*cout<<"Servicing multicast request : leader id  is : "<<Rxmsg.Access.LeaderUID;
        cout<<"map size is : "<<leader_client_info_map.size()<<endl;*/
        //map<string,clientinfo>::iterator it=leader_client_info_map.begin();
        //cout<<"client id : "<<it->first<<endl;
    }
    //RELEASE leader_send_receive_map MUTEX
    leader_send_receive_map_mutex.unlock();

    //RELEASE leader_client_info_map MUTEX
    leader_client_info_map_mutex.unlock();
    //RELEASE leader_client_info_map MUTEX
    /*Done seperately so that the iterator is not modified while accessing --- safe*/


}

void ServiceMULTICAST_REQUEST_ACK(recievedmsg UnpackedMsg){
    user Rxmsg;
    Rxmsg=DeserializeChatMsg(UnpackedMsg.Serialized_payload, delimiter);

    //check if the message received is from the same group
    if(groupID==Rxmsg.Access.GroupUID){

        //ACQUIRE client_send_map MUTEX
        client_send_map_mutex.lock();
        map<int,string>::iterator send_map_iterator = client_send_map.find(Rxmsg.Ack_No);
        if(send_map_iterator!=client_send_map.end()){
            client_send_map.erase(send_map_iterator);
            //cout<<"ServiceMULTICAST_REQUEST_ACK::entry deleted from client send map\n";
        }else{
            //cout<<"ServiceMULTICAST_REQUEST_ACK::message is already deleted\n";
        }
        //RELEASE client_send_map MUTEX
        client_send_map_mutex.unlock();
    }else{
        /*cout<<"ServiceMULTICAST_REQUEST_ACK::Not a the right group ... groupid is incorrect\n";
        cout<<"ServiceMULTICAST_REQUEST_ACK::group id received : "<<Rxmsg.Access.GroupUID<<endl;
        cout<<"ServiceMULTICAST_REQUEST_ACK::group id stored : "<<groupID<<endl;*/
    }


}

void ServiceMULTICAST_BROADCAST(recievedmsg UnpackedMsg){

    user Rxmsg;

    Rxmsg=DeserializeChatMsg(UnpackedMsg.Serialized_payload, delimiter);

    if(Rxmsg.Seq_No==1){
        //cout<<"entered the loop to reset\n";
        client_last_received_leader_seq=Rxmsg.Seq_No;
    }

    user Txmsg;
    string SerialisedString;

    //cout<<"Recieved MULTICAST_BROADCAST"<<"\n";
    //cout<<"MULTICAST_BROADCAST::Recieved Leader Sequnce Number:"<<Rxmsg.Seq_No<<"\n";
    //cout<<"Client Sequnce Number:"<<Rxmsg.Seq_No<<"\n\n\n";
    Txmsg.Msg_Type="MULTICAST_BROADCAST_ACK";
    Txmsg.client_name=chat_name;
    Txmsg.Seq_No=0;
    Txmsg.Ack_No=Rxmsg.Seq_No;
    Txmsg.Access.LeaderUID=leaderID;
    Txmsg.Access.ClientUID=clientID;
    Txmsg.Access.GroupUID=groupID;
    Txmsg.client.sin_family=AF_INET;
    inet_aton(leaderIP.c_str(),&Txmsg.client.sin_addr);
    Txmsg.client.sin_port=leaderport;
    Txmsg.user_message=Rxmsg.user_message;
    SerialisedString=SerializeChatMsg(Txmsg, delimiter);

    int sock;
    struct sockaddr_in sa;

    sock = socket(AF_INET,SOCK_DGRAM,0);
    if (-1 == sock) /* if socket failed to initialize, exit */
    {
        //cout<<"ServiceMULTICAST_BROADCAST::Error Creating Socket";
        exit(EXIT_FAILURE);
    }

    //Zero out socket address
    memset(&sa, 0, sizeof sa);

    //The address is ipv4
    sa.sin_family = AF_INET;


    //ip_v4 adresses is a uint32_t, convert a string representation of the octets to the appropriate value
    inet_aton(leaderIP.c_str(), &sa.sin_addr);
    // sa.sin_addr.s_addr = inet_addr(leaderIP.c_str()); //Need to get appropriate IP address from map

    //sockets are unsigned shorts, htons(x) ensures x is in network byte order, set the port to 7654
    sa.sin_port = htons(leaderport); //Need to get appropriate port address from map

    char buffer[65535];
    int length = SerialisedString.copy(buffer,SerialisedString.length(),0);
    buffer[length]='\0';
    //int status = sendto(socket_id,buffer,sizeof(buffer),0,(struct sockaddr *)&MaxClient->second.client,sizeof(MaxClient->second.client));


    int status = sendto(sock,buffer,strlen(buffer),0,(struct sockaddr *)&sa,sizeof(sa));
    if(status>0){
        //cout<<"ServiceMULTICAST_BROADCAST::send successful\n";
    }
    close(sock);

    //ACQUIRE client_recieve_map MUTEX
    client_receive_map_mutex.lock();
    client_send_map_mutex.lock();
    //cout<<"ServiceMULTICAST_BROADCAST::seq received : "<<Rxmsg.Seq_No<<endl;
    //cout<<"ServiceMULTICAST_BROADCAST::value of client_last_received_leader_seq : "<<client_last_received_leader_seq<<endl;
    if(firstPacket){
        //cout<<"ServiceMULTICAST_BROADCAST:: firstPacket seq received : "<<Rxmsg.Seq_No<<endl;
        //cout<<"ServiceMULTICAST_BROADCAST:: firstPacket value of client_last_received_leader_seq : "<<client_last_received_leader_seq<<endl;
        client_last_received_leader_seq=Rxmsg.Seq_No;
        pair<map<int,string>::iterator,bool> addToReceiveMap;
        addToReceiveMap=client_receive_map.insert(pair<int,string>(client_last_received_leader_seq,Rxmsg.user_message));
        if(addToReceiveMap.second == false)
        {
            cout<<"Error1: Leader sequence number already present in the map!!!"<<"\n";
        }
        firstPacket = false;
    }else{
        /*cout<<"ServiceMULTICAST_BROADCAST::seq received : "<<Rxmsg.Seq_No<<endl;
            cout<<"ServiceMULTICAST_BROADCAST::value of client_last_received_leader_seq : "<<client_last_received_leader_seq<<endl;*/
        if(Rxmsg.Seq_No == client_last_received_leader_seq+1){
            client_last_received_leader_seq=Rxmsg.Seq_No;
            pair<map<int,string>::iterator,bool> addToReceiveMap;
            addToReceiveMap=client_receive_map.insert(pair<int,string>(client_last_received_leader_seq,Rxmsg.user_message));

        }
        else{
            /*cout<<"ServiceMULTICAST_BROADCAST::failed to send\n";
        cout<<"ServiceMULTICAST_BROADCAST::seq received : "<<Rxmsg.Seq_No;
        cout<<"ServiceMULTICAST_BROADCAST::value of client_last_received_leader_seq : "<<client_last_received_leader_seq;*/
        }
    }
    client_send_map_mutex.unlock();
    client_receive_map_mutex.unlock();
    //RELEASE client_recieve_map MUTEX
}

void ServiceMULTICAST_BROADCAST_ACK(recievedmsg UnpackedMsg)
{
    user Rxmsg;


    Rxmsg=DeserializeChatMsg(UnpackedMsg.Serialized_payload, delimiter);
    //cout<<"Recieved MULTICAST_BROADCAST_ACK with ack no : "<<Rxmsg.Ack_No<<"\n";
    //ACQUIRE leader_client_info_map MUTEX
    leader_client_info_map_mutex.lock();
    //cout<<"current map size : "<<leader_client_info_map.size()<<endl;
    //Extract Recieved Clients information, Update ACK_No
    map<string,clientinfo>::iterator it=leader_client_info_map.find(Rxmsg.Access.ClientUID);                        //Access new clients info
    if(it!=leader_client_info_map.end())
    {
        it->second.ACK_No = Rxmsg.Ack_No;
        //cout<<"Recieved ACK No:"<< it->second.ACK_No<<"\n";
        //cout<<"ServiceMULTICAST_BROADCAST_ACK::updated the ack number\n";
    }else{
        //cout<<"ServiceMULTICAST_BROADCAST_ACK::failed to update the ack number\n";
    }
    leader_client_info_map_mutex.unlock();
    //RELEASE leader_client_info_map MUTEX

}

void ServiceLEAVE(recievedmsg UnpackedMsg)
{
    user Rxmsg, Txmsg;
    string SerialisedString;

    Rxmsg=DeserializeChatMsg(UnpackedMsg.Serialized_payload, delimiter);

    //ACQUIRE leader_client_info_map MUTEX
    leader_client_info_map_mutex.lock();
    leader_client_info_map.erase(Rxmsg.Access.ClientUID);
    leader_client_info_map_mutex.unlock();
    //RELEASE leader_client_info_map MUTEX

    Txmsg.Msg_Type="LEAVE_SUCCESS";
    Txmsg.Seq_No=0;
    Txmsg.Ack_No=0;
    Txmsg.Access.LeaderUID=leaderID;
    Txmsg.Access.ClientUID=Rxmsg.Access.ClientUID;
    Txmsg.Access.GroupUID=groupID;
    //Txmsg.client.sin_family=AF_INET;
    //Txmsg.client.sin_addr=leaderaddr;
    //Txmsg.client.sin_port=LeaderPort;
    Txmsg.user_message="Yo have been exterminated from the chat database";
    SerialisedString=SerializeChatMsg(Txmsg, delimiter);
    /*Send SerialisedString to socket. Address of client in sockaddr struct(client) of Rxmsg*/

}

void ServiceSERVICE(recievedmsg UnpackedMsg)
{
    leader_elect_proc=false;
    user Txmsg;
    string SerialisedString, RxLeaderID;

    //ACQUIRE client_info_map mutex
    client_info_map_mutex.lock();
    client_info_map = DeserializeMap(UnpackedMsg.Serialized_payload, delimiter);    //Copy entire client_info_map

    map<string,clientinfo>::iterator iterfindlip=client_info_map.begin();
    RxLeaderID=iterfindlip->second.CurrentleaderID;

    string splitlIP(RxLeaderID.c_str());
    string splitter = ":";
    leaderIP = splitlIP.substr(0, splitlIP.find(splitter));
    //cout<<"leader IP : "<<leaderIP<<endl;
    //cout<<"1. splitIP : "<<splitlIP<<endl;
    splitlIP.erase(0, leaderIP.length()+1);
    //cout<<"2. splitIP : "<<splitlIP<<endl;
    leaderport = atoi(splitlIP.c_str());

    client_info_map_mutex.unlock();
    //ACQUIRE client_info_map mutex

    Ldeadcount = 0;
    Txmsg.client_name = chat_name;
    Txmsg.Msg_Type="LEADERACK";
    Txmsg.Seq_No=0;
    Txmsg.Ack_No=0;
    Txmsg.Access.LeaderUID=leaderID;
    Txmsg.Access.ClientUID=clientID;
    Txmsg.Access.GroupUID=groupID;
    Txmsg.client.sin_family=AF_INET;
    inet_aton(clientIP.c_str(),&Txmsg.client.sin_addr);
    Txmsg.client.sin_port=clientport;
    Txmsg.user_message=" It's a message serviced by client to leader";
    SerialisedString=SerializeChatMsg(Txmsg, delimiter);



    int sock;
    struct sockaddr_in sa;

    sock = socket(AF_INET,SOCK_DGRAM,0);
    if (-1 == sock) // if socket failed to initialize, exit
    {
        //cout<<"ServiceSERVICE::Error Creating Socket";
        exit(EXIT_FAILURE);
    }

    //Zero out socket address
    memset(&sa, 0, sizeof sa);

    //The address is ipv4
    sa.sin_family = AF_INET;

    //ip_v4 adresses is a uint32_t, convert a string representation of the octets to the appropriate value
    sa.sin_addr.s_addr = inet_addr(leaderIP.c_str()); //Need to get appropriate IP address from map

    //sockets are unsigned shorts, htons(x) ensures x is in network byte order, set the port to 7654
    sa.sin_port = htons(leaderport); //Need to get appropriate port address from map

    char buffer[65535];
    int length = SerialisedString.copy(buffer,SerialisedString.length(),0);
    buffer[length]='\0';
    //int status = sendto(socket_id,buffer,sizeof(buffer),0,(struct sockaddr *)&MaxClient->second.client,sizeof(MaxClient->second.client));

    int status = sendto(sock,buffer,strlen(buffer),0,(struct sockaddr *)&sa,sizeof(sa));
    //cout<<"LEADERACK Sent to Leader IP:"<<inet_ntoa(sa.sin_addr)<<"\n";
    close(sock);

}

void ServiceLEADERACK(recievedmsg UnpackedMsg)
{
    //cout<<"Entering ServiceLEADERACK"<<"\n";
    user Rxmsg;

    Rxmsg = DeserializeChatMsg(UnpackedMsg.Serialized_payload,delimiter);
    clientID = Rxmsg.Access.ClientUID;

    //ACQUIRE leader_client_info_map MUTEX
    leader_client_info_map_mutex.lock();
    leader_client_info_map[clientID].DeadCount=0;
    leader_client_info_map_mutex.unlock();
    //RELEASE leader_client_info_map MUTEX

}

void ServiceNOTICE(recievedmsg UnpackedMsg)
{
    user Rxmsg;
    Rxmsg = DeserializeChatMsg(UnpackedMsg.Serialized_payload,delimiter);
    cout<<Rxmsg.user_message<<"\n"<<flush;
}

//// For Testing//////////////////////////////////////////////////////////////////////////////////
/*
void DisplayClientMap()
{
     cout<<"Client Map"<<"\n";
     cout<<"Key \t\t Value"<<"\n";
    for( map<int,clientinfo>::iterator it=leader_client_info_map.begin();it != leader_client_info_map.end(); it++)
    {
         cout << it->first << "\t=>" << "IsAlive" << it->second.IsAlive <<"\n";
         cout <<"\t"<< "ACK_No" <<"=>" << it->second.ACK_No <<"\n";
         cout <<"\t"<< "SEQ_No"<< "=>" << it->second.SEQ_No <<"\n";
         cout <<"\t"<< "Net Family"<< "=>" << it->second.client.sin_family <<"\n";
         cout <<"\t"<< "IP Address"<< "=>" << inet_ntoa(it->second.client.sin_addr) <<"\n";
         cout <<"\t"<< "Port No"<< "=>" << ntohs(it->second.client.sin_port) <<"\n";
         cout << "\n";
    }
     cout<<"\n";
}

void DisplayClientSendMap()
{
     cout<<"Send Map"<<"\n";
     cout<<"Seq_No \t\t Serialized_Message"<<"\n";
    for( map<int, string>::iterator itersend=client_send_map.begin(); itersend!=client_send_map.end(); itersend++)
    {
     cout<<itersend->first<<"\t\t"<<itersend->second<<"\n";
    }
     cout<<"\n";
}

user GenerateChatMessage()
{
     string ipaddress = "192.169.0.9";
    user chatpayloadtosend;
    chatpayloadtosend.Msg_Type="MULTICAST_REQUEST";
    chatpayloadtosend.Seq_No=1;
    chatpayloadtosend.Access.LeaderUID=1000;
    chatpayloadtosend.Access.ClientUID=23;
    chatpayloadtosend.Access.GroupUID=20;
    chatpayloadtosend.client.sin_family=AF_INET;
    inet_aton(ipaddress.c_str(),&chatpayloadtosend.client.sin_addr);
    chatpayloadtosend.client.sin_port=htons(3200);
    chatpayloadtosend.user_message="Hi, I have joined this amazing Chat system";
    return chatpayloadtosend;
}

void DisplayChatpayloadSendtruct(user chatpayloadtosend)
{
     cout<<"Displaying chat message Struct(to be sent)"<<"\n";
     cout<<"Message Type:"<<chatpayloadtosend.Msg_Type<<"\n";
     cout<<"Sequence No:"<<chatpayloadtosend.Seq_No<<"\n";
     cout<<"LeaderUID:"<<chatpayloadtosend.Access.LeaderUID<<"\n";
     cout<<"ClientUID:"<<chatpayloadtosend.Access.ClientUID<<"\n";
     cout<<"GroupUID:"<<chatpayloadtosend.Access.GroupUID<<"\n";
     cout<<"NET Family:"<<chatpayloadtosend.client.sin_family<<"\n";
     cout<<"Ip Address:"<<inet_ntoa(chatpayloadtosend.client.sin_addr)<<"\n";
     cout<<"Port Number:"<<ntohs(chatpayloadtosend.client.sin_port)<<"\n";
     cout<<"Chat Message:"<<chatpayloadtosend.user_message<<"\n";
}
*/
////////////////////////////////////////////////////////////////////////////////////////////////////

void *get_msg(void * threadid)
{

    //cout<<"Entered get_msg thread"<<"\n";
    long tid;
    tid = (long)threadid;

    struct sockaddr_in server;
    int socket_id,message_length;
    char buffer[65535];

    ////For Testing
    recievedmsg UnpackedMsg;
    user GeneratedMsg, RecievedMsg;
    //bool leader=true;

    // string RecievedSerialisedMsg;
    //GeneratedMsg=GenerateChatMessage();
    //DisplayChatpayloadSendtruct(GeneratedMsg);
    //RecievedSerialisedMsg=SerializeChatMsg(GeneratedMsg, delimiter);
    ///
    socket_id=socket(AF_INET,SOCK_DGRAM,0);

    //sets the value to 0 for the size of the serv
    memset((char *)&server,0,sizeof(server));
    //set the attributes
    server.sin_family = AF_INET;

    if(leader)
    {
        //cout<<"-----------------leader--------------------\n";
        server.sin_port=htons(leaderport);
        inet_aton(leaderIP.c_str(), &server.sin_addr);
        //server.sin_port=leaderport;
    }
    else if(client)
    {
        //cout<<"-----------------client--------------------\n";
        inet_aton(clientIP.c_str(), &server.sin_addr);
        server.sin_port=htons(clientport);

        //server.sin_port=htons(clientport);
    }
    //server.sin_port=0;

    //printf("Address : %s",inet_ntoa(server.sin_addr));

    //bind the socket created earlier
    int bind_value = bind(socket_id,(struct sockaddr *)&server,sizeof(server));
    if (bind_value < 0) {
        perror("bind failed");
        return 0;
    }
    while(1)
    {
        //cout<<"Waiting for input"<<"\n";
        message_length = recvfrom(socket_id,buffer,65535,0,NULL,NULL);
        //cout<<"Waited for input"<<"\n";
        buffer[message_length] = 0;
        string RecievedSerialisedMsg(buffer,message_length);

        // I have recieved RecievedSerialisedMsg from the socket

        //cout<<"--------------------------------------------------------\n";
        //cout<<"Rx Serialized Message:"<<RecievedSerialisedMsg<<"\n\n";

        UnpackedMsg=DeserializeMsgType(RecievedSerialisedMsg, delimiter);
        //cout<<"Message:";
        switch(Get_e(UnpackedMsg.Msg_Type))
        {
        // leader Zone
        case eJOIN:                     if(leader || client)
            {
                //cout<<"Servicing JOIN. . ."<<"\n";
                ServiceJOIN(UnpackedMsg);
            }
            else  {
                //cout<<"Not a leader Message";
            }
            break;

        case eMULTICAST_REQUEST:        if(leader)
            {
                //cout<<"Serviceing MULTICAST_REQUEST. . ."<<"\n";
                ServiceMULTICAST_REQUEST(UnpackedMsg);
            }
            else  {
                //cout<<"Not a leader Message";
            }
            break;

        case eMULTICAST_BROADCAST_ACK:  if(leader)
            {
                //cout<<"Serviceing MULTICAST_BROADCAST_ACK. . ."<<"\n";
                ServiceMULTICAST_BROADCAST_ACK(UnpackedMsg);
            }
            else  {
                //cout<<"Not a leader Message";
            }
            break;

        case eLEADERACK:                if(leader)
            {
                //cout<<"Serviceing LEADERACK. . .";
                ServiceLEADERACK(UnpackedMsg);
            }
            else  {
                //cout<<"Not a leader Message";
            }
            break;

        case eLEAVE:                    if(leader)
            {
                //cout<<"Serviceing LEAVE. . .";
                // ServiceLEAVE();
            }
            else  {
                //cout<<"Not a leader Message";
            }
            break;


        case eSERVICE:                  if(client)
            {
                //cout<<"Serviceing SERVICE. . .";
                ServiceSERVICE(UnpackedMsg);
            }
            else  {
                //cout<<"Not a client Message";
            }
            break;

            // Client Zone

        case eJOIN_ACK_MEMBER:          if(client)
            {
                //cout<<"Serviceing JOIN_ACK_MEMBER. . .";
                ServiceJOIN_ACK_MEMBER(UnpackedMsg);
            }
            else  {
                //cout<<"Not a client Message";
            }
            break;

        case eJOIN_ACK_LEADER:          if(client)
            {
                //cout<<"Serviceing JOIN_ACK_LEADER. . .";
                ServiceJOIN_ACK_LEADER(UnpackedMsg);
            }
            else  {
                //cout<<"Not a client Message";
            }
            break;

        case eMULTICAST_REQUEST_ACK:    if(client)
            {
                //cout<<"Serviceing MULTICAST_REQUEST_ACK. . .";
                ServiceMULTICAST_REQUEST_ACK(UnpackedMsg);
            }
            else  {
                //cout<<"Not a client Message";
            }
            break;

        case eMULTICAST_BROADCAST:      if(client)
            {
                //cout<<"Serviceing MULTICAST_BROADCAST. . ."<<"\n";
                ServiceMULTICAST_BROADCAST(UnpackedMsg);
            }
            else  {
                //cout<<"Not a client Message";
            }
            break;

        case eLEADER:                   if(client)
            {
                //cout<<"Serviceing LEADER. . .";
                ServiceLEADER(UnpackedMsg);
            }
            else
            {
                //cout<<"Recieved Leader Message But ";
                //cout<<"IsClient Rx LEADER message:"<<client<<"\n";
                //cout<<"IsLeader Rx LEADER message:"<<leader<<"\n";
                //cout<<"Not a client Message"<<"\n";
            }
            break;
        case eNOTICE:                   if(client)
            {
                //cout<<"Servicing NOTICE. . .";
                ServiceNOTICE(UnpackedMsg);
                break;
            }

        case eLEAVE_SUCCESS:            if(client)
            {
                //cout<<"Serviceing LEAVE_SUCCESS. . .";
                //ServiceLEAVE_SUCCESS();
            }
            else  {
                //cout<<"Not a client Message";
            }
            break;

        case INVALID:                   //cout<<"INVALID MESSAGE";
            break;

        default:                        //cout<<"INVALID CASE";
            break;
        }
        //cout<<"\n";
    }
    pthread_exit(NULL);
}

#endif
