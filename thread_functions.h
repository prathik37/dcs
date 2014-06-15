#pragma once
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <set>
#include "main.h"
#include "MessageSerialize.h"



void LeaderElectProtocol();

/*the get_chat_message is used from the MessageSerialize.h file
which performs the switching of the reeived messages*/

using namespace std;


//thread to read messages from UI and add to the map
void *chat_input(void *threadid)
{
    long tid;

    tid = (long)threadid;
    string Name_message;
    string bufferedInputString;

    while(1)
    {

        if(client){
            //initialize the seq no to 0 and later in the while loop it will keep incrementing and get added to the map
            //sclient_seq_no = 0;


            char str[65535];

            while(!feof(stdin) && fgets(str, 65535, stdin))
            {
               // fgets(str, 65535, stdin);
                string message(str);
                if(leader == true && client == false)
                {
                    //cout<<"Breaking from loop1"<<"\n";
                    bufferedInputString=message;
                    switchMade=true;
                    break;
                }

                if(message=="\n"){
                    cout<<"Enter a valid message to send!!!!\n"<<flush;
                }else{
                    //ACQUIRE client_send_map MUTEX
                    client_send_map_mutex.lock();
                        client_seq_no++;
                        //cout<<"Client Sequence Number:"<<client_seq_no<<"\n";
                        //add message to the map with the global sequence number and the message entered
                        user chatpayloadtosend;
                        string sendString;
                        chatpayloadtosend.Msg_Type="MULTICAST_REQUEST";
                        chatpayloadtosend.client_name=chat_name;
                        chatpayloadtosend.Seq_No=client_seq_no;
                        //cout<<"Client MULTICAST Sequence Number:"<<chatpayloadtosend.Seq_No<<"\n\n";
                        chatpayloadtosend.Ack_No=0;
                        chatpayloadtosend.Access.LeaderUID=leaderID;
                        chatpayloadtosend.Access.ClientUID=clientID;
                        chatpayloadtosend.Access.GroupUID=groupID;
                        chatpayloadtosend.client.sin_family=AF_INET;
                        inet_aton(clientIP.c_str(),&chatpayloadtosend.client.sin_addr);
                        chatpayloadtosend.client.sin_port=htons(clientport);

                        Name_message.append(chat_name);
                        Name_message.append(":: ");
                        Name_message.append(message);

                        chatpayloadtosend.user_message=Name_message;
                        sendString = SerializeChatMsg(chatpayloadtosend,delimiter);
                        /*cout<<"chatpayloadtosend.Access.LeaderUID : "<<chatpayloadtosend.Access.LeaderUID<<endl;
                                cout<<"chatpayloadtosend.Access.ClientUID : "<<chatpayloadtosend.Access.ClientUID<<endl;
                                cout<<"chatpayloadtosend.Access.GroupUID : "<<chatpayloadtosend.Access.GroupUID<<endl;*/
                        /*set the message which needs to be sent to the leader, i.e the serialised message*/

                        client_send_map.insert (pair<int,string>(client_seq_no,sendString));
                        //cout<<"size of client map : "<<client_send_map.size()<<endl;
                    client_send_map_mutex.unlock();
                    //RELEASE client_send_map MUTEX
                    Name_message.clear();
                }
            }

            if(!leader && client)
            {
                //cout<<"Chat ends here"<<endl;
                exit(0);
            }


        }

        if(leader){
            //initialize the seq no to 0 and later in the while loop it will keep incrementing and get added to the map
            //leader_seq_no = 0;

            char str[65535];

            if(switchMade){
                if(!bufferedInputString.empty()){
                    if(bufferedInputString=="\n"){
                        cout<<"Enter a valid message to send!!!!\n"<<flush;
                    }else{
                        //ACQUIRE leader_send_receive_map MUTEX
                        leader_send_receive_map_mutex.lock();
                        //ACQUIRE leader_client_info_map MUTEX
                        leader_client_info_map_mutex.lock();

                        Name_message=chat_name;
                        Name_message.append(":: ");
                        Name_message.append(bufferedInputString);

                        if(!leader_client_info_map.empty())
                        {
                            //cout<<"Entered if1";
                            leader_seq_no++;
                            //cout<<"Leader Sequence Number:"<<leader_seq_no<<"\n";
                            //Broadcast Packet
                            user broadcastPacket;
                            string broadcastPacketString;
                            broadcastPacket.Msg_Type="MULTICAST_BROADCAST";
                            broadcastPacket.client_name=chat_name;
                            broadcastPacket.Seq_No=leader_seq_no;
                            //cout<<"Broadcast Sequence Number:"<<broadcastPacket.Seq_No<<"\n\n";
                            broadcastPacket.Ack_No=0;
                            broadcastPacket.Access.LeaderUID=leaderID;
                            broadcastPacket.Access.ClientUID="NULL";
                            broadcastPacket.Access.GroupUID=groupID;
                            broadcastPacket.client.sin_family=AF_INET;
                            inet_aton(leaderIP.c_str(),&broadcastPacket.client.sin_addr);
                            broadcastPacket.client.sin_port=leaderport;


                            broadcastPacket.user_message=Name_message;
                            broadcastPacketString=SerializeChatMsg(broadcastPacket, delimiter);
                            //add message to the map with the global sequence number and the message entered
                            leader_send_receive_map.insert (pair<int,string>(leader_seq_no,broadcastPacketString));
                            //cout<<"current size of leader map : "<<leader_send_receive_map.size()<<endl;

                          }
                            else
                            {
                                //cout<<"Entered else1";
                                //cout<<Name_message;
                            }
                        leader_client_info_map_mutex.unlock();
                        //RELEASE leader_client_info_map MUTEX
                        leader_send_receive_map_mutex.unlock();
                        //RELEASE leader_send_receive_map MUTEX
                        Name_message.clear();
                    }
                    switchMade=false;
                    bufferedInputString.clear();
                }
            }

            while(!feof(stdin) && fgets(str, 65535, stdin))
            {
                //fgets(str, 65535, stdin);
                string message(str);
                if(leader == false && client == true)
                {
                   // cout<<"Breaking from loop2"<<"\n";
                    break;
                }
                if(message=="\n"){
                    cout<<"Enter a valid message to send!!!!\n"<<flush;
                }else{
                    //ACQUIRE leader_send_receive_map MUTEX
                    leader_send_receive_map_mutex.lock();
                    //ACQUIRE leader_client_info_map MUTEX
                    leader_client_info_map_mutex.lock();

                    Name_message=chat_name;
                    Name_message.append(":: ");
                    Name_message.append(message);
                    //cout<<"Entered valid message if"<<"\n";
                        if(!leader_client_info_map.empty())
                        {
                            //cout<<"Entered if2"<<"\n";
                            leader_seq_no++;
                            //cout<<"Leader Sequence Number:"<<leader_seq_no<<"\n";
                            //Broadcast Packet
                            user broadcastPacket;
                            string broadcastPacketString;
                            broadcastPacket.Msg_Type="MULTICAST_BROADCAST";
                            broadcastPacket.client_name=chat_name;
                            broadcastPacket.Seq_No=leader_seq_no;
                            //cout<<"Broadcast Sequence Number:"<<broadcastPacket.Seq_No<<"\n\n";
                            broadcastPacket.Ack_No=0;
                            broadcastPacket.Access.LeaderUID=leaderID;
                            broadcastPacket.Access.ClientUID="NULL";
                            broadcastPacket.Access.GroupUID=groupID;
                            broadcastPacket.client.sin_family=AF_INET;
                            inet_aton(leaderIP.c_str(),&broadcastPacket.client.sin_addr);
                            broadcastPacket.client.sin_port=leaderport;

                            broadcastPacket.user_message=Name_message;
                            broadcastPacketString=SerializeChatMsg(broadcastPacket, delimiter);
                            //add message to the map with the global sequence number and the message entered


                            leader_send_receive_map.insert (pair<int,string>(leader_seq_no,broadcastPacketString));
                        }
                        else
                        {
                           // cout<<"Entered else2"<<"\n";
                            //cout<<Name_message;
                        }

                        //cout<<"current size of leader map : "<<leader_send_receive_map.size()<<endl;

                    leader_client_info_map_mutex.unlock();
                    //RELEASE leader_client_info_map MUTEX
                    leader_send_receive_map_mutex.unlock();
                    //RELEASE leader_send_receive_map MUTEX
                    Name_message.clear();
                }
            }
            if(leader && !client)
            {
                //cout<<"Chat ends here"<<endl;
                exit(0);
            }


        }
    }
    pthread_exit(NULL);
}

void *send_msg(void *threadid)
{
    long tid;

    tid = (long)threadid;
    while(1){
        if(client){


            int sock;
            struct sockaddr_in sa;

            sock = socket(AF_INET,SOCK_DGRAM,0);
            //cout<<"value of sock : "<<sock<<endl;
            if (-1 == sock) /* if socket failed to initialize, exit */
            {
                //cout<<"send_msg::Error Creating Socket";
                exit(EXIT_FAILURE);
            }

            //Zero out socket address
            memset(&sa, 0, sizeof sa);



            while(1)
            {
                if(leader_elect_proc)
                {
                    //cout<<"Leader Electin going on1...";
                    usleep(300000);
                }

                if(leader == true && client == false)
                {
                    //cout<<"send_msg .. inner loop : Breaking from loop3"<<"\n";
                    break;
                }
                //The address is ipv4
                sa.sin_family = AF_INET;


                //ip_v4 adresses is a uint32_t, convert a string representation of the octets to the appropriate value
                inet_aton(leaderIP.c_str(), &sa.sin_addr);
                // sa.sin_addr.s_addr = inet_addr(leaderIP.c_str()); //Need to get appropriate IP address from map

                //sockets are unsigned shorts, htons(x) ensures x is in network byte order, set the port to 7654
                sa.sin_port = htons(leaderport); //Need to get appropriate port address from map

                //iterate over map entries and send the messages to the leader

                /*if(client_send_map.size()>300){
                cout<<"client map size : "<<client_send_map.size()<<endl;
            }*/

                //ACQUIRE client_send_map mutex
                client_send_map_mutex.lock();
                    map<int,string>::iterator send_map_iterator = client_send_map.begin();

                    for (send_map_iterator=client_send_map.begin(); send_map_iterator!=client_send_map.end(); ++send_map_iterator)
                    {
                        /*Extension --- Have an extern variable to not sent an already acknowldeged packet
                            Ideally --- It would be removed when the ack is received by the recv from thread*/
                        char buffer[65535];
                        int length = send_map_iterator->second.copy(buffer,send_map_iterator->second.length(),0);
                        buffer[length]='\0';
                        /*cout<<"buffer : \n\n"<<buffer<<endl;
                            cout<<"string : \n\n"<<send_map_iterator->second<<endl;*/
                        //for(int i=0; i<5;i++){
                        int status = sendto(sock,buffer,strlen(buffer),0,(struct sockaddr *)&sa,sizeof(sa));
                        //}
                        //usleep(10000);
                        break;
                        //cout<<"map size : "<<client_send_map.size()<<endl;
                        //cout<<"In client_send_message thread function \n Sequence Number sent : "<<send_map_iterator->first<<" and message sent size : "<<status<<endl;
                        //cout<<"Message Sent:"<<send_map_iterator->second.c_str()<<"\n";

                    }
                client_send_map_mutex.unlock();
                //RELEASE client_send_map mutex

                //needs to be checked .. should sleep for a while otherwise it will bombard leader with messages
                //need some time for recieving the acknowledgement and clearing the send map
                //sleep(1);

                usleep(100000);
            }


        }

        if(leader){
            int sock = socket(AF_INET, SOCK_DGRAM, 0);
            while(1)
            {
                if(leader_elect_proc)
                {
                    //cout<<"Leader Electin going on2...";
                    usleep(300000);
                }
                if(leader == false && client == true)
                {
                    //cout<<"Breaking from loop4"<<"\n";
                    break;
                }

                if(leader_elected){
                   // cout<<"entered slowdown loop\n\n\n";
                    sleep(1);
                    leader_elected=false;
                }
                //cout<<"thread client send message called\n";

                //sendto(int socket, char data, int dataLength, flags, destinationAddress, int destinationStructureLength)

                //iterate over map entries and send the messages to the leader

                //ACQUIRE leader_send_receive_map MUTEX
                leader_send_receive_map_mutex.lock();

                //ACQUIRE leader_client_info_map MUTEX
                leader_client_info_map_mutex.lock();




                //require cleanup from leader_send_receive map
                int min_ack_received = 0;
                set<int> min_ack_finder_set;


                if(!leader_send_receive_map.empty()){
                map<int,string>::iterator it=leader_send_receive_map.begin();
                    //cout<<"leader_send_receive_map not empty"<<endl;
                    //cout<<"current value of seq number in map : "<<it->first<<"\n\n";
                }

                //cout<<"leader leader_last_seq_no_printed : "<<leader_last_seq_no_printed<<endl;
                //cout<<"leader leader_last_deleted_message : "<<leader_last_deleted_message<<endl;

                if(leader_last_seq_no_printed>leader_last_deleted_message){
                  //  cout<<"Entered if 1"<<"\n";
                    if(!leader_client_info_map.empty()){
                    //    cout<<"Entered if 2"<<"\n";

                       // if(leader_client_info_map.empty())
                        //{
                      //      cout<<"leader_client_info_map id empty"<<"\n";
                        //}
                        map<string,clientinfo>::iterator leader_client_info_iterator_1 = leader_client_info_map.begin();

                        for (leader_client_info_iterator_1=leader_client_info_map.begin(); leader_client_info_iterator_1!=leader_client_info_map.end(); ++leader_client_info_iterator_1)
                        {
                            min_ack_finder_set.insert(leader_client_info_iterator_1->second.ACK_No);
                        }
                    }else{
                        //cout<<"Thread send_msg::map empty\n";
                    }


                    if(!min_ack_finder_set.empty()){
                        set<int>::iterator it=min_ack_finder_set.begin();
                        min_ack_received = *it;
                        //cout<<"value of min ack received, outside the if : "<<min_ack_received<<endl;
                        if(min_ack_received!=0 && min_ack_received>leader_last_deleted_message){
                            int start = leader_last_deleted_message + 1;
                            int test_delete_count = 0;
                          //  cout<<"value of start : "<<start<<endl;
                          //  cout<<"value of min ack received, inside the if : "<<min_ack_received<<endl;
                            while(start<=min_ack_received && leader_last_seq_no_printed>leader_last_deleted_message){
                                map<int,string>::iterator it=leader_send_receive_map.find(start);                        //Access new clients info
                                if(it!=leader_send_receive_map.end())
                                {
                                    //cout<<"Thread send_msg::entry being deleted : "<<start<<endl;
                                    leader_send_receive_map.erase(it);
                                    leader_last_deleted_message++;
                                    start++;
                                    test_delete_count++;
                                }else{
                                    //cout<<"breaking from the if cond\n";
                                    break;
                                }

                            }
                            //cout<<"Thread send_msg::entries deleted : "<<test_delete_count<<endl;
                        }else{
                            //cout<<"Thread send_msg::here\n";
                            /* cout<<"Thread send_msg::min_ack_received : "<<min_ack_received<<endl;
                            cout<<"Thread send_msg::leader_last_deleted_message : "<<leader_last_deleted_message<<endl;
                            cout<<"Thread send_msg::min_ack_finder_set size : "<<min_ack_finder_set.size()<<endl;*/
                        }

                    }else{
                        //cout<<"Thread send_msg::set empty\n";
                    }
                }

                if(!leader_send_receive_map.empty())
                {
                    //cout<<"leader_send_receive_map not empty II"<<endl;
                }

                    map<int,string>::iterator leader_send_map_iterator = leader_send_receive_map.begin();
                    map<string,clientinfo>::iterator leader_client_info_iterator = leader_client_info_map.begin();

                    for (leader_send_map_iterator=leader_send_receive_map.begin(); leader_send_map_iterator!=leader_send_receive_map.end(); ++leader_send_map_iterator)
                    {
                        /*Extension --- Have an extern variable to not sent an already acknowldeged packet
                                Ideally --- It would be removed when the ack is received by the recv from thread*/

                        /*access client info map to access all clients and sent the information
                            Handle deletion of messages when received an acknowledgment*/

                        for (leader_client_info_iterator=leader_client_info_map.begin(); leader_client_info_iterator!=leader_client_info_map.end(); ++leader_client_info_iterator)
                        {
                            char buffer[65535];
                            int length = leader_send_map_iterator->second.copy(buffer,leader_send_map_iterator->second.length(),0);
                            buffer[length]='\0';
                            //cout<<"leader map size : "<<leader_send_receive_map.size()<<endl;
                            int status = sendto(sock,buffer,strlen(buffer),0,(struct sockaddr *)&leader_client_info_iterator->second.client,sizeof(leader_client_info_iterator->second.client));
                            //cout<<"In client_send_message thread function \n Sequence Number sent : "<<leader_send_map_iterator->first<<" and message sent size : "<<status<<endl;

                        }
                        break;
                    }



                //RELEASE leader_client_info_map MUTEX
                leader_client_info_map_mutex.unlock();

                //RELEASE leader_send_receive_map MUTEX
                leader_send_receive_map_mutex.unlock();

                //needs to be checked .. should sleep for a while otherwise it will bombard leader with messages
                //need some time for recieving the acknowledgement and clearing the send map
                //sleep(1);

                usleep(100000);
            }
        }

    }
    pthread_exit(NULL);

}

void *print_msg(void *threadid){

    long tid;
    tid = (long)threadid;

    while(1){

        if(client){
            while(1)
            {
                    if(leader_elect_proc)
                    {
                        //cout<<"Leader Electin going on1...";
                        usleep(300000);
                    }
                    if(leader == true && client == false)
                    {
                        //cout<<"Breaking from loop5"<<"\n";
                        break;
                    }
                //find the the next entry in the map to print, i.e. the next entry


                /*map<int,string>::iterator receive_map_iterator = client_receive_map.find(client_last_seq_no_printed+1);
            if(receive_map_iterator!=client_receive_map.end()){
                //cout<<"Leader Sequence Number : "<<receive_map_iterator->first<<"\n";
                //cout<<"Message received : "<<receive_map_iterator->second<<"\n";
                client_last_seq_no_printed = receive_map_iterator->first;
                cout<<receive_map_iterator->second;
            }*/

                //ACQUIRE client_receive_map MUTEX
                client_receive_map_mutex.lock();

                    set<int> erase_set;
                    if(!client_receive_map.empty()){
                        map<int,string>::iterator receive_map_iterator = client_receive_map.begin();
                        if(receive_map_iterator!=client_receive_map.end()){
                            for (receive_map_iterator=client_receive_map.begin(); receive_map_iterator!=client_receive_map.end(); ++receive_map_iterator)
                            {
                                if(receive_map_iterator->first>client_last_seq_no_printed){
                                    client_last_seq_no_printed = receive_map_iterator->first;
                                    cout<<receive_map_iterator->second<<flush;
                                    erase_set.insert(client_last_seq_no_printed);
                                }
                            }
                        }

                    }


                    //cleanup of the map
                    if(!erase_set.empty()){
                        for (set<int>::iterator it=erase_set.begin(); it!=erase_set.end(); ++it){
                            client_receive_map.erase(*it);
                        }
                    }

                    client_receive_map_mutex.unlock();
                //RELEASE client_receive_map MUTEX

                usleep(10000);
            }

        }

        if(leader){
            while(1)
            {
                if(leader_elect_proc)
                {
                    //cout<<"Leader Electin going on3...";
                    usleep(300000);
                }

                if(leader == false && client == true)
                {
                    //cout<<"Breaking from loop6"<<"\n";
                    break;
                }
                //ACQUIRE leader_send_receive_map MUTEX
                leader_send_receive_map_mutex.lock();

                    if(!leader_send_receive_map.empty()){
                        map<int,string>::iterator receive_map_iterator = leader_send_receive_map.find(leader_last_seq_no_printed+1);
                        if(receive_map_iterator!=leader_send_receive_map.end()){
                            /*cout<<"___Entered leader_last_seq_no_printed+1"<<"\n";
                            cout<<"Leader Sequence Number : "<<receive_map_iterator->first<<"\n";
                            cout<<"Message received : "<<receive_map_iterator->second<<"\n";*/
                            leader_last_seq_no_printed = receive_map_iterator->first;
                            //cout<<"__leader_last_seq_no_printed"<<leader_last_seq_no_printed<<"\n";
                            user received = DeserializeChatMsg(receive_map_iterator->second,delimiter);
                            cout<<received.user_message<<flush;
                        }
                        //else
                            //cout<<"receive_map_iterator==leader_send_receive_map.end()"<<"\n";
                    }
                    //else
                        //cout<<"leader_send_receive_map.empty()"<<"\n";

                leader_send_receive_map_mutex.unlock();
                //RELEASE leader_send_receive_map MUTEX

                usleep(70000);
            }

        }

    }
    pthread_exit(NULL);
}





//this thread sends the entire client_info map to everyone in the map.
void *leader_election(void *threadid)
{

    long tid;
    tid = (long)threadid;
    ostringstream ostrnotice;
    string NoticeMessage;
    user TxNotice;

    while(1)
    {
        if(leader){

            int sock,sock_notice;
            sock = socket(AF_INET,SOCK_DGRAM,0);
            sock_notice = socket(AF_INET,SOCK_DGRAM,0);

            while(1)
            {
                //cout<<"I am leader will send service"<<"\n\n";

                //ACQUIRE leader_client_info_map MUTEX
                leader_client_info_map_mutex.lock();
                    map<string,clientinfo>::iterator leader_client_info_iterator = leader_client_info_map.begin();
                    //find the the next entry in the map to print, i.e. the next entry

                    for (leader_client_info_iterator=leader_client_info_map.begin(); leader_client_info_iterator!=leader_client_info_map.end(); ++leader_client_info_iterator)
                    {

                        struct sockaddr_in sa;

                        sa = leader_client_info_iterator->second.client;

                        //ADD leadermap_version to serialization into leader_client_info_map
                        //string leaderversion = String.valueOf(leadermap_version);




                        leader_client_info_map[leader_client_info_iterator->first].DeadCount++;

                        if(leader_client_info_iterator->second.DeadCount>=5)
                        {
                            ostrnotice.str("");
                            ostrnotice.clear();
                            ostrnotice << "NOTICE " << leader_client_info_iterator->second.name << " left the chat or crashed" << "\n";
                            cout<<ostrnotice.str()<<flush;
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
                                int status = sendto(sock_notice,buffer,strlen(buffer),0,(struct sockaddr *)&notice_iterator->second.client,sizeof(notice_iterator->second.client));
                                //cout<<"In client_send_message thread function \n Sequence Number sent : "<<leader_send_map_iterator->first<<" and message sent size : "<<status<<endl;

                            }
                            map<string,clientinfo>::iterator leader_client_info_delete_iterator = leader_client_info_iterator;
                            leader_client_info_map.erase(leader_client_info_delete_iterator->first);
                            //cout<<"Client is dead!!"<<"\n";
                        }


                        //make SerializeMap from int to string

                        if(!leader_client_info_map.empty())
                        {
                            for(map<string,clientinfo>::iterator itersend=leader_client_info_map.begin(); itersend != leader_client_info_map.end(); ++itersend)
                            {
                                //cout<<"the leader sequence number added before sending : " <<leader_seq_no<<endl;
                                itersend->second.Extra=leader_seq_no;
                            }
                                string SerializedMap_String = SerializeMap(leader_client_info_map,delimiter);

                                char buffer[65535];
                                int length = SerializedMap_String.copy(buffer,SerializedMap_String.length(),0);
                                buffer[length]='\0';
                                /*cout<<"buffer : \n\n"<<buffer<<endl;
                                cout<<"string : \n\n"<<send_map_iterator->second<<endl;*/
                                //cout<<"Sending Service Messages to all"<<"\n\n";
                                int status = sendto(sock,buffer,strlen(buffer),0,(struct sockaddr *)&sa,sizeof(sa));
                        }
                        //int status = sendto(sock,SerializedMap_String.c_str(),SerializedMap_String.length(),0,(struct sockaddr *)&sa,sizeof(sa));


                    }
                leader_client_info_map_mutex.unlock();
                //RELEASE leader_client_info_map MUTEX
                if(leader == false && client == true)
                {
                    //cout<<"Breaking from loop7"<<"\n";
                    break;
                }
                //sleep(1);
                usleep(10000);
            }
            // while(j<=sizeof(deletearray) && leader_client_info_map.size()>0)
            // leader_client_info_map.erase(deletearray[j++]);


            //usleep(100000); //sleep in microseconds


        }

        if(client){
            while(1)
            {
                Ldeadcount++;
                //cout<<"LeaderDeadCount:"<<Ldeadcount<<"\n";

                if(leader == true && client == false)
                {
                    //cout<<"Breaking from loop8"<<"\n";
                    break;
                }
                if(Ldeadcount>=5)
                {
                    //cout<<"Leader is dead, re-election initiated\n";
                    LeaderElectProtocol();
                    Ldeadcount = 0;
                }
                //sleep(1);
                usleep(100000);
            }
        }
    }

    pthread_exit(NULL);

}

void LeaderElectProtocol()
{
    //cout<<"Entered Leader ElectProtocol"<<"\n";
    //cout<<"IsClient Before Election:"<<client<<"\n";
    //cout<<"IsLeader Before Election:"<<leader<<"\n";

    /////
    /*Reset client state*/
    //1)
            firstPacket=true;

    //2)    //Flush all elements to screen to empty it
            //ACQUIRE client_receive_map MUTEX
            client_receive_map_mutex.lock();

                    if(!client_receive_map.empty())
                    {
                        set<int> erase_set;
                        if(!client_receive_map.empty()){
                            map<int,string>::iterator receive_map_iterator = client_receive_map.begin();
                            if(receive_map_iterator!=client_receive_map.end()){
                                for (receive_map_iterator=client_receive_map.begin(); receive_map_iterator!=client_receive_map.end(); ++receive_map_iterator)
                                {
                                    //cout<<"Flushing recieve map..."<<"\n";
                                        cout<<receive_map_iterator->second<<flush;
                                        erase_set.insert(client_last_seq_no_printed);

                                }
                            }

                        }


                        //cleanup of the map
                        if(!erase_set.empty()){
                            for (set<int>::iterator it=erase_set.begin(); it!=erase_set.end(); ++it){
                                client_receive_map.erase(*it);
                            }
                        }
                        //cout<<"Done Flushing recieve map..."<<"\n";
                    }
                    else
                    {
                        //cout<<"Client Recieve map is empty"<<"\n";
                    }

            client_receive_map_mutex.unlock();
        //RELEASE client_receive_map MUTEX

    //3)
            client_last_seq_no_printed=0;
            client_last_received_leader_seq=0;
            //client_seq_no=0;



    int ip1,ip2,ip3,ip4,iport;
    long max_IPPORT=0,sum_IPPORT;
    map<string,clientinfo>::iterator MaxClient;
    ostringstream ostrip;

    string SerialisedString;
    user Txmsg;
    string ipstr, portstr;
    leader_elect_proc=true;
    doneOnce=false;

    //cout<<"printing Summer Map:"<<"\n";
    //Find Client with max(IP+Port)

    //ACQUIRE client_info_map mutex
    client_info_map_mutex.lock();

        if(!client_info_map.empty()){
            for(map<string,clientinfo>::iterator itmax=client_info_map.begin();itmax !=client_info_map.end();++itmax)
            {
                //////////////////////////
                /*std::cout << itmax->first << "\t=>" << itmax->second.name <<"\n";
                std::cout << "\t\t\t=>" << itmax->second.DeadCount <<"\n";
                std::cout << "\t\t\t=>" << itmax->second.Extra <<"\n";
                std::cout << "\t\t\t=>" << itmax->second.IsAlive <<"\n";
                std::cout << "\t\t\t=>" << itmax->second.ACK_No <<"\n";
                std::cout << "\t\t\t=>" << itmax->second.SEQ_No <<"\n";
                std::cout << "\t\t\t=>" << itmax->second.client.sin_family <<"\n";
                std::cout << "\t\t\t=>" << inet_ntoa(itmax->second.client.sin_addr)<<"\n";
                std::cout << "\t\t\t=>" << ntohs(itmax->second.client.sin_port) <<"\n";
                std::cout << "\n";*/
                //////////////////////////
                ostrip <<inet_ntoa(itmax->second.client.sin_addr);
                ipstr=ostrip.str();
                ostrip.str("");
                ostrip.clear();
                sscanf(ipstr.c_str(),"%d.%d.%d.%d", &ip1,&ip2,&ip3,&ip4);
                ostrip << ntohs(itmax->second.client.sin_port);
                portstr=ostrip.str();
                ostrip.str("");
                ostrip.clear();
                sscanf(portstr.c_str(),"%d", &iport);
                sum_IPPORT=ip1+ip2+ip3+ip4+iport;
                if(max_IPPORT < sum_IPPORT)
                {
                    max_IPPORT=sum_IPPORT;
                    MaxClient=itmax;
                }
                //cout<<"Test1"<<"\n";
            }

            /*cout<<"max_IPPORT:"<<max_IPPORT<<"\n";
            //cout<<"MaxClient.Access.CLientUID"<<MaxClient->second.IsAlive<<"\n";*/
            //Send Successor invite to be Leader
            Txmsg.Msg_Type="LEADER";
            Txmsg.client_name=chat_name;
            Txmsg.Seq_No=0;
            Txmsg.Ack_No=0;
            Txmsg.Access.LeaderUID="";
            Txmsg.Access.ClientUID=clientID;
            Txmsg.Access.GroupUID="";
            Txmsg.client.sin_family=AF_INET;
            inet_aton(clientIP.c_str(),&Txmsg.client.sin_addr);
            Txmsg.client.sin_port=htons(clientport);
            Txmsg.user_message="You are the Leader!";

            // Make propsed Successor as leader(for now)
            leaderIP=(inet_ntoa(MaxClient->second.client.sin_addr));
            leaderID=MaxClient->first;
            leaderport=ntohs(MaxClient->second.client.sin_port);
            //cout<<"New LeaderID:"<<leaderID<<"\n";
            // Send Message
            SerialisedString=SerializeChatMsg(Txmsg, delimiter);
            int socket_id=socket(AF_INET,SOCK_DGRAM,0);

            char buffer[65535];
            int length = SerialisedString.copy(buffer,SerialisedString.length(),0);
            buffer[length]='\0';
            /*cout<<"buffer : \n\n"<<buffer<<endl;
            //cout<<"string : \n\n"<<send_map_iterator->second<<endl;*/
            int status = sendto(socket_id,buffer,strlen(buffer),0,(struct sockaddr *)&MaxClient->second.client,sizeof(MaxClient->second.client));


            //sendto(socket_id,SerialisedString.c_str(),SerialisedString.length(),0,(struct sockaddr *)&MaxClient->second.client,sizeof(MaxClient->second.client));
            //cout<<"Sent Leader Election message:"<<SerialisedString.c_str()<<"\n";

            //Delete Leader Entry
            client_info_map.erase(MaxClient);
            if(client_info_map.empty())
            {
                //cout<<"Map Empty"<<"\n";
            }


        //RELEASE client_info_map mutex

        //leader_elect_proc=false;


        //cout<<"Election complete\n\n\n";
    }else{
            cout<<"Sorry, no chat is active on "<<leaderID<<", try again later.\nBye.\n";
            exit(0);

    }
        client_info_map_mutex.unlock();

    //cout<<"IsClient After Election:"<<client<<"\n";
    //cout<<"IsLeader After Election:"<<leader<<"\n";
    return;
}
