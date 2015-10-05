
#ifndef SOCKET_H
#define SOCKET_H

#include <sys/socket.h>
#include <string>

using namespace std;

enum SocketType
{
    ST_TCP = 1,
    ST_UDP
};


class Socket
{
    public:
        Socket(){}
        Socket(const char* ip, int port, SocketType type);
        ~Socket();

        int Create();
        int Bind();
        int Connect();

        int Listen(int backlog = 1000);
        int Accept(Socket* client);

        int Send(const void* buffer, unsigned int size);
        int Recv(const void* buffer, unsigned int size);

        int Close();

        void SetSocket(int sockId){m_iSocket = sockId;}
        int GetSocket(){return m_iSocket;}
        int GetPeerName(struct sockaddr *name, socklen_t *namelen);
	int SetSockOpt(int level, int optname, const void *optval, socklen_t optlen);
	int SetSockAddressReuse(bool reuse = true);

        string GetIp();
        int GetPort();

    private:
        int m_iSocket;
        int m_iPort;
        const char* m_pIp;
        int m_nSocketType;
};




#endif
