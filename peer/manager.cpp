#include <unistd.h>
#include <errno.h>
#include <iostream>
#include <string.h>
#include <string>
#include <cstdlib>
#include "manager.h"
#include <dirent.h>
#include <sys/epoll.h>
#include "tools.h"
#include <fstream>
#include <sys/time.h>
#include <arpa/inet.h>
#include <signal.h>
#include <assert.h>

using namespace std;

Manager::Manager(string configfile)
{
	m_iPid = 0;
	m_pSocket = NULL;
	m_pClientSock = NULL;
	m_semRequest = NULL;
	m_mtxRequest = NULL;
	m_pUserProcess = NULL;

	Config* config = Config::Instance();
	if( config->ParseConfig(configfile.c_str(), "SYSTEM") != 0)
	{
		cout<<"parse config file:["<<configfile<<"] failed, exit"<<endl;
		exit(-1);
	}


	m_iServernum = config->GetIntVal("SYSTEM", "servernum", 8);
	m_iCurrentServernum = config->GetIntVal("SYSTEM", "currentservernum", 0);
	assert(m_iCurrentServernum < m_iServernum);
	m_iPeerThreadPoolNum = config->GetIntVal("SYSTEM", "threadnum", 5);
	m_iTestMode = config->GetIntVal("SYSTEM", "testmode", 0);

	for(int i = 0; i < m_iServernum; i++)
	{
		char serverip[30] = {0};
		char serverport[30] = {0};
		char serveridentifier[30] = {0};
		snprintf(serverip, 30, "serverip_%d", i);
		snprintf(serverport, 10, "serverport_%d", i);
		snprintf(serveridentifier, 10, "server_identifier_%d", i);
		PeerInfo pi;
		pi.ip = config->GetStrVal("SYSTEM", serverip, "0.0.0.0");
		pi.port = config->GetIntVal("SYSTEM", serverport, 55555);
		pi.identifier = config->GetIntVal("SYSTEM", serveridentifier, 0);
		m_vecPeerInfo.push_back(pi);
	}

	m_strPeerIp = m_vecPeerInfo[m_iCurrentServernum].ip;
	m_iPeerPort = m_vecPeerInfo[m_iCurrentServernum].port;


}

Manager::~Manager()
{
	if(m_pClientSock)
	{
		delete m_pClientSock;
		m_pClientSock = NULL;
	}
	if(m_pSocket)
	{
		delete m_pSocket;
		m_pSocket = NULL;
	}
	if(m_semRequest)
	{
		delete m_semRequest;
		m_semRequest = NULL;
	}
	if(m_mtxRequest)
	{
		delete m_mtxRequest;
		m_mtxRequest = NULL;
	}
	for(unsigned int i = 0; i < m_vecProcessThread.size(); i++)
	{
		if(m_vecProcessThread[i])
		{
			delete m_vecProcessThread[i];
			m_vecProcessThread[i] = NULL;
		}
	}
	if(m_pUserProcess)
	{
		delete m_pUserProcess;
		m_pUserProcess = NULL;
	}
}

int Manager::Start()
{
	m_iPid = fork();
	if(m_iPid == -1)
	{
		cout<<"fork failed"<<endl;
		return -1;
	}
	else if (m_iPid == 0)
	{
		if(Init() < 0)
		{
			cout<<"manager init failed"<<endl;
			return 0;
		}
		if(Listen() < 0)
		{
			cout<<"manager listen failed"<<endl;
			return -1;
		}
		Loop();
	}
	return 0;
}

int  Manager::IsStoped()
{
	int result = ::kill(m_iPid, 0);
	if (0 == result || errno != ESRCH)
	{
		return false;
	}
	else	
	{
		m_iPid = 0;
		return true;
	}
}

int Manager::Init()
{
	m_pSocket = new Socket(m_strPeerIp.c_str(), m_iPeerPort, ST_TCP);
	m_semRequest = new Sem(0, 0);
	m_mtxRequest = new Mutex();
	for(int i = 0; i <= m_iPeerThreadPoolNum; i++)
	{
		Thread* thread = new Thread(Process, this);
		m_vecProcessThread.push_back(thread);
	}

	m_pUserProcess = new Thread(UserCmdProcess, this);

	return 0;
}

int Manager::Listen()
{
	if (m_pSocket->Create() < 0)
	{
		cout<<"socket create failed"<<endl;
		return -1;
	}
	if(m_pSocket->SetSockAddressReuse(true) < 0)
		cout<<"set socket address reuse failed"<<endl;
	if(m_pSocket->Bind() < 0)
	{
		cout<<"socket bind failed"<<endl;
		return -1;
	}
	if(m_pSocket->Listen() < 0)
	{
		cout<<"socket listen failed"<<endl;
		return -1;
	}
	cout<<"now begin listen"<<endl;
	return 0;
}

int Manager::Loop()
{
	int listenfd = m_pSocket->GetSocket();
	struct epoll_event ev, events[MAX_EPOLL_FD];
	int epfd = epoll_create(MAX_EPOLL_FD);
	ev.data.fd = listenfd;
	ev.events = EPOLLIN;
	epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev);
	while(1)
	{
		int nfds = epoll_wait(epfd, events, MAX_EPOLL_FD, -1);
		if(nfds <= 0) continue;
		for (int i = 0; i < nfds; i++)
		{
			Socket* s = new Socket();
			int iRet = m_pSocket->Accept(s);
			if(iRet < 0)
			{
				cout<<"socket accept failed: ["<<errno<<"]"<<endl;
				continue;
			}
			s->SetSockAddressReuse(true);
			m_mtxRequest->Lock();
			m_rq.push(s);
			m_semRequest->Post();
			m_mtxRequest->Unlock();
		}
		usleep(100000);
	}
return 0;
}

void* Process(void* arg)
{
return 0;
}


void* UserCmdProcess(void* arg)
{
	return 0;
}






