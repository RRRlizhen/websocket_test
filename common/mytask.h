#ifndef __RECVANDSENDTASK_H
#define __RECVANDSENDTASK_H

#include "myepoll.h"
#include "basetask.h"
#include <string.h>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <stdint.h>

using namespace std;

class CTcpTask : public CBaseTask
{
public:
	CTcpTask(int ipSocket, CEpoll* epoll);
	virtual ~CTcpTask();

        //int DestroySelf();

		void SetAddr(sockaddr_in &addr)
		{
			m_SrcAddr = addr;
		}

		sockaddr_in& GetAddr()
		{
			return m_SrcAddr;
		}


protected:      

	int RecvProcess();

    int SendData();
	
	int ProcessOnePacket();	
	int PacketFinished();
	enum State
	{
		ReadPacketLen = 0,
		ReadPacket
	};

	CVBuffer  m_vReceiveBuffer;
	short        m_iDataSize;
	int		  m_iTotalPackLen;

	State m_state;
	sockaddr_in m_SrcAddr;			// client addr
};

class CLongConTask : public CTcpTask
{
public:
	CLongConTask(const string &sAddr, CEpoll* epoll, int nTimeOut = 2);

	CLongConTask(int ipSocket, CEpoll* epoll);

	virtual ~CLongConTask(){};

public:
	
	int Init();

	int Connect(const string &sAddr, int nTimeOut);

	string m_toAddr;
	int  m_iTimeOut;
	int m_toLen;
};

class CAcceptTask : public CBaseTask
{
public:
	CAcceptTask(const string &sAddr, CEpoll* epoll);

	CAcceptTask(){};
	virtual ~CAcceptTask(){};

	int HandleProcess(int event);

	int CreateListenSocket();

	sockaddr_un& GetUnAddr()
	{
		return m_ListenAddr;
	}

protected:

	sockaddr_un m_ListenAddr;
	string m_sAddr;
};

class CUdpTask : public CBaseTask
{

public:

	CUdpTask(const string &sAddr, int port, CEpoll* epoll)
	{
		m_iSocket = -1;
		m_pEpoll = epoll;
		m_sAddr = sAddr;
		m_iPort = port;
	}

	CUdpTask(){};

	virtual ~CUdpTask(){};

	virtual int ProcessOnePacket();

	virtual int PacketFinished();

	int  CreateBindSocket();

	sockaddr_in& GetAddr()
	{
		return m_BindAddr;
	}

protected:        
	int RecvProcess();

	int SendData();

	string  m_sAddr;
	int     m_iPort;

	CVBuffer  m_vReceiveBuffer;

	unsigned short m_iDataSize;
	sockaddr_in m_BindAddr;		  // bind addr

public:

	sockaddr_in m_fromAddr;		  //  recvfrom addr
	socklen_t   m_fromLen;
};


#endif
