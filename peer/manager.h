#ifndef MANAGER_H
#define MANAGER_H

#include "socket.h"
#include <queue>
#include "thread.h"
#include <vector>
#include "model.h"
#include <string.h>
#include "config.h"
#include "hashManager.h"

using namespace std;

typedef queue<Socket*> RequestQueue;
//process function for each thread
void* Process(void* arg);

void* UserCmdProcess(void* arg);

class Manager
{
	friend void* Process(void* arg);  // thread pool to handler all the hash request
	friend void* UserCmdProcess(void* arg); //single thread to handler user input
public:
	Manager(string configfile = "../config/client.config");
	~Manager();

	int Start();
	int IsStoped();

protected:
	int Init();
	int Listen();
	int Loop();

private:
	int put(string key, string value);
	int get(string key, string& value);
	bool del(string key);
	// for uniform distribution, we here suppose the key is all number string, like "999"
	// and getHash only return its int.
	int getHash(const string& key);

	int testmode();

	Socket* getSock(string ip, int port);


private:
	Socket* m_pClientSock;
	Socket* m_pSocket;
	RequestQueue m_rq;

	HashtableManager m_htm;

	string m_strSelfIp;
	int m_iSelfPort;

	int m_iServernum;  // total number of peers exist in this static network
	int m_iCurrentServernum;  // the current server(peer) identifier in this static network
	int m_iPeerThreadPoolNum; // thread num
	int m_ihashnum;
	int m_iTestMode; 

	vector<PeerInfo> m_vecPeerInfo; // all peers in this static network info

	Sem* m_semRequest;
	Mutex* m_mtxRequest;

	vector<Thread*> m_vecProcessThread;
	Thread* m_pUserProcess;
	
	int m_iPid;


	unsigned int m_iPutTime;
	unsigned int m_iGetTime;
	unsigned int m_iDelTime;

};






#endif
