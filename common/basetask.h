
#ifndef __BASE_TASK_H
#define __BASE_TASK_H
//#include "myepoll.h"
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <list>
#include <fcntl.h>
#include <netinet/in.h>
#include <vector>
#include "common.h"
#include <stdint.h>
#include "buffer.h"

class CEpoll;
using namespace std;

class CSendMsg
{
public:
	CSendMsg(int len, const char *buf)
	{
		m_sendLen = len;
		m_sendBody = (char*) new char[m_sendLen];
		memcpy(m_sendBody, buf, len);	
	};

	virtual ~CSendMsg()
	{
		if (m_sendBody != NULL)
		{
			delete[]m_sendBody;
		}
	};

	unsigned int GetLen()
	{
		return m_sendLen;
	};

	char *GetBody()
	{
		return m_sendBody;
	};

	void SetAddr(string sAddr, uint16_t iPort)
	{

		sockaddr_in tmpAddr;
		memset((char *)&tmpAddr, 0, sizeof(tmpAddr));
		tmpAddr.sin_port = htons(iPort);
		inet_aton(sAddr.c_str(), &tmpAddr.sin_addr);
		memcpy(&m_toAddr, &tmpAddr, sizeof(sockaddr_in));
		m_toLen = sizeof(sockaddr_in);

	}

	void SetAddr(uint32_t iAddr, uint16_t iPort)
	{
		memset((char *)&m_toAddr, 0, sizeof(m_toAddr));

		m_toAddr.sin_family = AF_INET;
		m_toAddr.sin_addr.s_addr = iAddr;
		m_toAddr.sin_port = htons(iPort);
		m_toLen = sizeof(m_toAddr);

	}

	void SetAddr(sockaddr_in SoAddr)
	{
		m_toAddr = SoAddr;
		m_toLen = sizeof(m_toAddr);
	}

	sockaddr_in& GetAddr()
	{
		return m_toAddr;
	}

	socklen_t GetAddrLen()
	{
		return m_toLen;
	}
protected:
	unsigned int m_sendLen;
	char* m_sendBody;

	sockaddr_in m_toAddr;
	socklen_t    m_toLen;
};


class CBaseTask
{
public:
	CBaseTask(int isocket, CEpoll* epoll);
	
    CBaseTask();

	virtual ~CBaseTask();

	virtual int Init();

	virtual int HandleProcess(int events);

	virtual int DestroySelf();

	int DelFromEpoll();

	virtual int CloseConnect();

	static int SetNonBlocking(int nSocket);

	int GetSocket()
	{
		return m_iSocket;
	}

	virtual void SetAddr(string sAddr, string sPort)
	{
	}

	virtual void SetAddr(sockaddr_in& addrInfo)
	{
		
	}

	virtual sockaddr_in& GetAddr()
	{
		assert(false);
		static sockaddr_in noused;
		return noused;
	}

	int PutMsgToSendList(CSendMsg* );


protected:        
	virtual int RecvProcess(){assert(false);return 0;};

	virtual int SendProcess();

	virtual int SendData(){assert(false);return 0;};

	int  AddToEpoll();         

	CSendMsg *GetMsgFromSendList();


	virtual int ProcessOnePacket(){assert(false);return 0;};

	virtual int PacketFinished() {assert(false);return 0;};
	list<CSendMsg*> m_sendList;
	unsigned int m_iCurSendLen;
	CSendMsg* m_pCurSendMsg;
	int m_nSendNum;
	int		  m_nRecvNum;

	int		m_iSocket;
	CEpoll* m_pEpoll;
};

#endif
