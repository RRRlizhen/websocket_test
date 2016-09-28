#include "basetask.h"
#include "myepoll.h"
#include <errno.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include "writeRunInfo.h"

const int DEFAULT_SEND_NUM = 20;
const int DEFAULT_RECV_NUM = 2;

CBaseTask::CBaseTask(int iSocket, CEpoll* epoll)
{        
	m_nRecvNum = DEFAULT_RECV_NUM;
	m_nSendNum = DEFAULT_SEND_NUM;
	m_iCurSendLen = 0;
	m_pCurSendMsg = NULL;
	m_pEpoll = epoll;
	m_iSocket = iSocket;
		
}

CBaseTask::CBaseTask()
{
    m_nRecvNum = DEFAULT_RECV_NUM;
    m_nSendNum = DEFAULT_SEND_NUM;
    m_iCurSendLen = 0;
    m_pCurSendMsg = NULL;
    m_pEpoll = NULL;
    m_iSocket = -1;
}

CBaseTask::~CBaseTask()
{
	if (m_pCurSendMsg != NULL)
		delete m_pCurSendMsg;

	list<CSendMsg*>::iterator listItr = m_sendList.begin();
	for (; listItr != m_sendList.end();)
	{
		list<CSendMsg*>::iterator tmpItr = listItr;
		listItr++;
		delete *tmpItr;
		m_sendList.erase(tmpItr);
	}
	if (m_iSocket != -1)
	{
		close(m_iSocket);
		m_iSocket = -1;
	}
}

int CBaseTask::Init()
{
	AddToEpoll();
	return 0;
}


int CBaseTask::DestroySelf()
{
	CloseConnect();
	delete this;
	return 0;
}

int CBaseTask::CloseConnect()
{
	DelFromEpoll();
	if (m_iSocket != -1)
	{
		WRITE_INFO("close connect fd %d ip %s port %d\n", m_iSocket, inet_ntoa(GetAddr().sin_addr), ntohs(GetAddr().sin_port));
		struct linger ln;
		ln.l_onoff = 1;
		ln.l_linger = 0;
		if (setsockopt(m_iSocket, SOL_SOCKET, SO_LINGER, (void*)&ln, sizeof(ln)) < 0)
		{
			WRITE_ERROR("setsockopt(SO_LINGER) %d %s", errno, strerror(errno));
		}

		close( m_iSocket);
		m_iSocket = -1;
	}
	return 0;
}


int CBaseTask::DelFromEpoll()
{
	if (m_iSocket == -1)
	{
		return 0;
	}

	m_pEpoll->DelFromEpoll(this);

	return 0;
}

int CBaseTask::HandleProcess(int events)
{        
	if ((events & EPOLLERR) == EPOLLERR || (events & EPOLLHUP) == EPOLLHUP)
	{
		events = events| EPOLLIN | EPOLLOUT; // ģ��Nginx ��д��   ��������IO ����  socket����
	}

	int ret = 0;
	if ((events & EPOLLIN) == EPOLLIN) //���¼�
	{
		ret = RecvProcess();
		if (ret == -1)
		{
			DestroySelf(); 
			return -1;
		}
	}

	if ((events & EPOLLOUT) == EPOLLOUT) //д�¼�
	{
		ret =  SendProcess(); 
		if ( ret == -1 )
		{
			DestroySelf();
			return -1;                	
		}
	}               
	return 0;
}

int CBaseTask::AddToEpoll()
{          
	int tmpEvent = EPOLLERR|EPOLLHUP|EPOLLIN;

	if (m_sendList.size() != 0 || m_pCurSendMsg != NULL)
	{
		tmpEvent = tmpEvent|EPOLLOUT;
	}

	m_pEpoll->AddToEpoll(this, tmpEvent);
	return 0;
}


int CBaseTask::SendProcess()
{
	int retValue = 0;
	int ii = 0;
	while (ii < m_nSendNum)									 // ÿһ�� ��෢�� 20����Ϣ
	{

		if (m_pCurSendMsg != NULL)
		{
			retValue = SendData();							// �ȷ��͵�ǰû�з���������ݰ�
		}
		else
		{
			m_pCurSendMsg = GetMsgFromSendList();			 // �ӷ����б���ȡ��ʣ�µ����ݰ� ��������
			if (m_pCurSendMsg != NULL)
			{
				retValue = SendData();
			}
			else											 // û������Ҫ���� ��ɾ���� EPOLLOUT �¼�  ��ֹ��ͣ�ĳ���EPOLLOUT�Ļص�
			{
				WRITE_INFO("CBaseTask::SendProcess Del EPOLLOUT\n");
				int tmpEvent = EPOLLERR | EPOLLHUP | EPOLLIN;
				m_pEpoll->ModifyFromEpoll(this, tmpEvent);
				retValue = 2;								// û��Ҫ���͵�������
			}
		}

		if (retValue == 2 || retValue == -1 )				// û��Ҫ���͵����� ���� ���� ���˳�
			break;
		ii++;
	}

	return retValue;
}

CSendMsg *CBaseTask::GetMsgFromSendList()
{
	if (m_sendList.begin() == m_sendList.end())
	{
		return NULL;
	}
	list<CSendMsg*>::iterator itr = m_sendList.begin();
	CSendMsg *pMsg = (*itr);
	m_sendList.erase(itr);
	return pMsg;                
}

int CBaseTask::PutMsgToSendList(CSendMsg *pBuf)
{        
	if (pBuf == NULL || pBuf->GetLen() == 0)
	{       
		return -1;
	}               

	m_sendList.push_back(pBuf);

	if (!m_sendList.empty())
	{
		WRITE_INFO("CBaseTask::PutMsgToSendList Add EPOLLOUT fd:%d\n", m_iSocket);
		int tmpEvent = EPOLLERR|EPOLLHUP|EPOLLIN|EPOLLOUT;
		m_pEpoll->ModifyFromEpoll(this, tmpEvent);
	}
	return 0;
}

int CBaseTask::SetNonBlocking(int nSocket)
{
	int opts;
	opts=fcntl(nSocket, F_GETFL);
	if(opts<0)
	{
		return -1;
	}
	opts = opts|O_NONBLOCK;
	opts = opts|O_NDELAY;
	if(fcntl(nSocket,F_SETFL,opts)<0)
	{
		return -1;
	}

	int  nSendBuf=100*1024;//����Ϊ100K  
	setsockopt(nSocket,SOL_SOCKET,SO_SNDBUF,(const  char*)&nSendBuf,sizeof(int));  
	setsockopt(nSocket,SOL_SOCKET,SO_RCVBUF,(const  char*)&nSendBuf,sizeof(int));   	
	int on = 1;
	setsockopt(nSocket, SOL_TCP, TCP_NODELAY,  &on, sizeof(on));
	return 0;
}

