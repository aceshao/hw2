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
	m_ihashnum = config->GetIntVal("SYSTEM", "hashnum", 100000);
	m_iPeerThreadPoolNum = config->GetIntVal("SYSTEM", "threadnum", 5);
	m_iTestMode = config->GetIntVal("SYSTEM", "testmode", 0);

	m_htm = HashtableManager((unsigned int)m_ihashnum);

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

	m_strSelfIp = m_vecPeerInfo[m_iCurrentServernum].ip;
	m_iSelfPort = m_vecPeerInfo[m_iCurrentServernum].port;


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
	m_pSocket = new Socket(m_strSelfIp.c_str(), m_iSelfPort, ST_TCP);
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
	Manager* pmgr = (Manager*)arg;

	while(1)
	{
		pmgr->m_semRequest->Wait();
		pmgr->m_mtxRequest->Lock();
		Socket* client = m_rq.front();
		pmgr->m_rq.pop();
		pmgr->m_mtxRequest->Unlock();

		char* recvBuff = new char[MAX_MESSAGE_LENGTH];
		bzero(recvBuff, MAX_MESSAGE_LENGTH);
		if(client->Recv(recvBuff, MAX_MESSAGE_LENGTH) != MAX_MESSAGE_LENGTH)
		{
			cout<<"recv failed"<<endl;
			client->Close();
			delete [] recvBuff;
			continue;
		}

		char* sendBuff = new char[MAX_MESSAGE_LENGTH];
		bzero(sendBuff, MAX_MESSAGE_LENGTH);
		Message* sendMsg = (Message*)sendBuff;

		Message* recvMsg = (Message*)recvBuff;
		switch(recvMsg->action)
		{
			case CMD_SEARCH:
			{
				string value = "";
				if(pmgr->m_htm.Search(recvMsg->key, value) != 0)
				{
					// search failed, return the value null
					sendMsg->action = CMD_OK;
					strncpy(sendMsg->key, recvMsg->key, MAX_KEY_LENGTH);
					bzero(sendMsg->value, MAX_VALUE_LENGTH);
				}
				else
				{
					sendMsg->action = CMD_OK;
					strncpy(sendMsg->key, recvMsg->key, MAX_KEY_LENGTH);
					strncpy(sendMsg->value, value.c_str(), MAX_KEY_LENGTH);
				}
				client->Send(sendMsg, MAX_MESSAGE_LENGTH);
				break;
			}

			case CMD_PUT:
			{
				if(pmgr->m_htm.Insert(recvMsg->key, recvMsg->value) != 0)
				{
					sendMsg->action = CMD_FAILED;
					strncpy(sendMsg->key, recvMsg->key, MAX_KEY_LENGTH);
					strncpy(sendMsg->value, recvMsg->value, MAX_VALUE_LENGTH);
				}
				else
				{
					sendMsg->action = CMD_OK;
					strncpy(sendMsg->key, recvMsg->key, MAX_KEY_LENGTH);
					strncpy(sendMsg->value, recvMsg->value, MAX_VALUE_LENGTH);
				}
				client->Send(sendMsg, MAX_MESSAGE_LENGTH);
				break;				
			}
			case CMD_DEL:
			{
				if(pmgr->m_htm.Delete(recvMsg->key) != 0)
				{
					sendMsg->action = CMD_FAILED;
					strncpy(sendMsg->key, recvMsg->key, MAX_KEY_LENGTH);
					bzero(sendMsg->value, MAX_VALUE_LENGTH);
				}
				else
				{
					sendMsg->action = CMD_OK;
					strncpy(sendMsg->key, recvMsg->key, MAX_KEY_LENGTH);
					bzero(sendMsg->value, MAX_VALUE_LENGTH);
				}
				client->Send(sendMsg, MAX_MESSAGE_LENGTH);
				break;				
			}
			default:
			cout<<"cmd unknown["<<recvMsg->action<<"]"<<endl;
		}

		client->Close();
		delete[] recvBuff;
		delete[] sendBuff;
	}

	return 0;
}


void* UserCmdProcess(void* arg)
{
	Manager* pmgr = (Manager*)arg;

	cout<<"Welcome to the hash distributed system, you are in the client"<<endl;
	cout<<"You can put, get, del key to and from the system"<<endl;
	while(1)
	{
		cout<<"Presee 1 to put, 2 to get, 3 to del"<<endl;
		int action = 0;
		cin>>action;
		if(action == 1)
		{
			string key = "";
			string value = "";
			cout<<"Please enter the key"<<endl;
			cin>>key;
			cout<<"Please enter the value"<<endl;
			cin>>value;

			if(pmgr->put(key, value) != 0)
				cout<<"put failed"<<endl;
			else
				cout<<"put success"<<endl;

		}
		else if(action == 2)
		{
			string key = "";
			string value = "";
			cout<<"Please enter the key"<<endl;
			cin>>key;

			if(pmgr->get(key) != 0 || value == "")
				cout<<"get failed"<<endl;
			else
				cout<<"get success"<<endl;

		}
		else if(action == 3)
		{
			string key = "";
			cout<<"Please enter the key"<<endl;
			cin>>key;

			if(pmgr->del(key) != 0)
				cout<<"del failed"<<endl;
			else
				cout<<"del success"<<endl;
		}
		else
		{
			cout<<"The action you typed not recognized, please confirm"<<endl;
		}
	}
	return 0;
}


int Manager::put(const string& key, const string& value)
{
	int hash = getHash(key);
	string severip = m_vecPeerInfo[hash%m_iServernum].ip;
	int serverport = m_vecPeerInfo[hash%m_iServernum].port;
	Socket* sock = new Socket(severip.c_str(), serverport, ST_TCP);
	sock->Create();

	if(sock->Connect() != 0)
	{
		cout<<"connect to hash server failed"<<endl;
		delete sock;
		return -1;
	}

	char* sbuff = new char[MAX_MESSAGE_LENGTH];
	bzero(sbuff, MAX_MESSAGE_LENGTH);
	(Message*) smsg = (Message*)sbuff;
	smsg->action = CMD_PUT;
	strncpy(smsg->key, key.c_str(), MAX_KEY_LENGTH);
	strncpy(smsg->value, value.c_str(), MAX_VALUE_LENGTH);

	if(sock->Send(sbuff, MAX_MESSAGE_LENGTH) != MAX_MESSAGE_LENGTH)
	{
		cout<<"send put message to hash server failed"<<endl;
		sock->Close();
		delete sock;
		delete[] sbuff;
		return -1;
	}

	char* rbuff = new char[MAX_MESSAGE_LENGTH];
	bzero(rbuff, MAX_MESSAGE_LENGTH);
	if(sock->Recv(rbuff, MAX_MESSAGE_LENGTH) != MAX_MESSAGE_LENGTH)
	{
		cout<<"put message recv from hash server failed"<<endl;
		sock->Close();
		delete sock;
		delete[] sbuff;
		delete[] rbuff;
		return -1;
	}

	(Message*) rmsg = (Message*)rbuff;
	int ret = rmsg->action == CMD_OK?0:-1;

	sock->Close();
	delete sock;
	delete[] sbuff;
	delete[] rbuff;
	return ret;
}
int Manager::get(const string& key, string& value)
{
	int hash = getHash(key);
	string severip = m_vecPeerInfo[hash%m_iServernum].ip;
	int serverport = m_vecPeerInfo[hash%m_iServernum].port;
	Socket* sock = new Socket(severip.c_str(), serverport, ST_TCP);
	sock->Create();

	if(sock->Connect() != 0)
	{
		cout<<"connect to hash server failed"<<endl;
		delete sock;
		return -1;
	}

	char* sbuff = new char[MAX_MESSAGE_LENGTH];
	bzero(sbuff, MAX_MESSAGE_LENGTH);
	(Message*) smsg = (Message*)sbuff;
	smsg->action = CMD_SEARCH;
	strncpy(smsg->key, key.c_str(), MAX_KEY_LENGTH);

	if(sock->Send(sbuff, MAX_MESSAGE_LENGTH) != MAX_MESSAGE_LENGTH)
	{
		cout<<"send search message to hash server failed"<<endl;
		sock->Close();
		delete sock;
		delete[] sbuff;
		return -1;
	}

	char* rbuff = new char[MAX_MESSAGE_LENGTH];
	bzero(rbuff, MAX_MESSAGE_LENGTH);
	if(sock->Recv(rbuff, MAX_MESSAGE_LENGTH) != MAX_MESSAGE_LENGTH)
	{
		cout<<"search message recv from hash server failed"<<endl;
		sock->Close();
		delete sock;
		delete[] sbuff;
		delete[] rbuff;
		return -1;
	}

	(Message*) rmsg = (Message*)rbuff;
	value = rmsg->value;

	sock->Close();
	delete sock;
	delete[] sbuff;
	delete[] rbuff;
	return 0;
}

bool Manager::del(const string& key)
{
	int hash = getHash(key);
	string severip = m_vecPeerInfo[hash%m_iServernum].ip;
	int serverport = m_vecPeerInfo[hash%m_iServernum].port;
	Socket* sock = new Socket(severip.c_str(), serverport, ST_TCP);
	sock->Create();

	if(sock->Connect() != 0)
	{
		cout<<"connect to hash server failed"<<endl;
		delete sock;
		return -1;
	}

	char* sbuff = new char[MAX_MESSAGE_LENGTH];
	bzero(sbuff, MAX_MESSAGE_LENGTH);
	(Message*) smsg = (Message*)sbuff;
	smsg->action = CMD_DEL;
	strncpy(smsg->key, key.c_str(), MAX_KEY_LENGTH);

	if(sock->Send(sbuff, MAX_MESSAGE_LENGTH) != MAX_MESSAGE_LENGTH)
	{
		cout<<"send put message to hash server failed"<<endl;
		sock->Close();
		delete sock;
		delete[] sbuff;
		return -1;
	}

	char* rbuff = new char[MAX_MESSAGE_LENGTH];
	bzero(rbuff, MAX_MESSAGE_LENGTH);
	if(sock->Recv(rbuff, MAX_MESSAGE_LENGTH) != MAX_MESSAGE_LENGTH)
	{
		cout<<"put message recv from hash server failed"<<endl;
		sock->Close();
		delete sock;
		delete[] sbuff;
		delete[] rbuff;
		return -1;
	}

	(Message*) rmsg = (Message*)rbuff;
	int ret = rmsg->action == CMD_OK?0:-1;

	sock->Close();
	delete sock;
	delete[] sbuff;
	delete[] rbuff;
	return ret;
}

int Manager::getHash(const string& key)
{
	return atoi(key.c_str());
}


