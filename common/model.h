#ifndef MODEL_H
#define MODEL_H

#include <string>
#include <vector>
using namespace std;

const int MAX_EPOLL_FD = 30;

enum Cmd_Indicate
{
	CMD_OK = 0,
	CMD_FAILED = 1,
	CMD_SEARCH = 2,
	CMD_PUT = 3,
	CMD_DEL = 4,
	CMD_UNKNOWN = 5
};

const int MAX_INDICATE_LENGTH = sizeof(Cmd_Indicate);
const int MAX_KEY_LENGTH = 20;
const int MAX_VALUE_LENGTH = 1000;
const int MAX_MESSAGE_LENGTH = MAX_KEY_LENGTH + MAX_VALUE_LENGTH + MAX_INDICATE_LENGTH;

typedef struct PeerInfo
{
	string ip;
	int port;
	int identifier;
	int keybegin;
	int keyend;

}PeerInfo;

#pragma pack(1)
typedef struct Message
{
	Cmd_Indicate action;
	char key[MAX_KEY_LENGTH];
	char value[MAX_VALUE_LENGTH];
}Message;

#pragma pack()

#endif
