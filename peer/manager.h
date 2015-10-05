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
	Socket* m_pClientSock;
	Socket* m_pSocket;
	RequestQueue m_rq;


	string m_strPeerIp;
	int m_iPeerPort;

	int m_iServernum;
	int m_iCurrentServernum;
	int m_iPeerThreadPoolNum;
	int m_iTestMode; 

	vector<PeerInfo> m_vecPeerInfo;

	Sem* m_semRequest;
	Mutex* m_mtxRequest;

	vector<Thread*> m_vecProcessThread;
	Thread* m_pUserProcess;
	
	int m_iPid;

};






#endif
