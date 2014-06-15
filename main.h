#pragma once
#include <map>
#include <string>
#include "structs.h"

using namespace std;

extern mutex client_send_map_mutex;
extern mutex client_receive_map_mutex;
extern mutex client_info_map_mutex;
extern mutex leader_send_receive_map_mutex;
extern mutex leader_client_info_map_mutex;


//client
extern bool client;
//key --- client message sequence number, value --- message payload
extern map<int,string> client_send_map;

//key --- leader message sequence number, value --- message payload
extern map<int,string> client_receive_map;

//to be used for printinf from the map
extern int client_last_seq_no_printed;

extern int client_last_received_leader_seq;

//global sequence number for entered messages in the UI
extern int client_seq_no;

//client map version number
extern int clientmap_version;

//client info map
extern map<string,clientinfo> client_info_map;

extern bool firstPacket;

//leader
extern bool leader;
//key --- leader message sequence number, value --- message payload
extern map<int,string> leader_send_receive_map;

//key --- leader message sequence number, value --- message payload
extern map<string,clientinfo> leader_client_info_map;

//to be used for printinf from the map
extern int leader_last_seq_no_printed;

//global sequence number for entered messages in the UI
extern int leader_seq_no;

extern int leader_last_deleted_message;

//leader map version number
extern int leadermap_version;

//client ID and client port
extern string clientID;
extern int clientport;
extern string clientIP;

//leader ID
extern string leaderID;
extern int leaderport;
extern string leaderIP;

//group ID - reverse of leaderID
extern string groupID;

extern string delimiter;

//Leader dead count
extern int Ldeadcount;

extern bool leader_elect_proc;

extern string chat_name;

extern bool leader_elected;

extern bool doneOnce;

extern bool switchMade;
