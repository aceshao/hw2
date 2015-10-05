#ifndef MODEL_H
#define MODEL_H

#include <string>
#include <vector>
using namespace std;

const int MAX_EPOLL_FD = 30;

typedef struct PeerInfo
{
	string ip;
	int port;
	int identifier;

}PeerInfo;

#endif
