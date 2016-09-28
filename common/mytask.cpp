#include "mytask.h"
#include <errno.h>
#include "myepoll.h"
#include <stdio.h>
#include "basetask.h"
#include <unistd.h>
#include "common.h"
#include <arpa/inet.h>
#include <string>
#include <sys/un.h>
#include "writeRunInfo.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

using namespace std;

const int DEFAULT_SEND_NUM = 1;
const int DEFAULT_RECV_NUM = 1;
const int DEFAULT_RECV_BUF = 32 * 1024;

CTcpTask::CTcpTask(int iSocket, CEpoll* epoll):CBaseTask(iSocket, epoll)
{        
		m_state = ReadPacketLen;
		m_iDataSize = 0;
		m_iTotalPackLen = 0;
}

CTcpTask::~CTcpTask()
{
		
}

int CTcpTask::PacketFinished()
{
	return (m_iTotalPackLen <= m_vReceiveBuffer.getSize()) ? 1 : 0;
}

int CTcpTask::ProcessOnePacket()
{
	//2(headlen)+headmsg+2(datalen)+datamsg
	// 解析数据

	CSendMsg* pMsg = new CSendMsg(m_iDataSize, m_vReceiveBuffer.JumpBuffer());

	m_vReceiveBuffer.Jump(m_iDataSize);
	m_vReceiveBuffer.AlignBuf();

	CMyserver::GetInstance()->PutMsgToList(pMsg);
	return 0;

}

int CTcpTask::RecvProcess()
{
	unsigned int uMaxReceive;
	int iReced;

	//2(totallen)+2(headlen)+headmsg+2(datalen)+datamsg

	WRITE_INFO("CTcpTask::RecvProcess   Recv from Client  fd %d\n", GetSocket());
	int ii = 0;
	do						
	{
		uMaxReceive = MAX_CVBUFFER_SIZE - m_vReceiveBuffer.getSize();

		iReced = recv(GetSocket(), m_vReceiveBuffer.JumpBuffer(), uMaxReceive, MSG_DONTWAIT);
		if (iReced == 0)
		{
			if (uMaxReceive != 0)
			{
				WRITE_INFO("RecvProcess client close the socket NeedSize= %d  iSocket= %d\n ", uMaxReceive, GetSocket());
				return -1;
			}
			else
			{
				WRITE_INFO("m_vReceiveBuffer Is Full go go go go \n");
			}
		}

		if (iReced < 0)
		{
			if (errno == EAGAIN || errno == EINTR)
			{
				return 0;
			}
			else
			{
				WRITE_ERROR("RecvProcess recv error uMaxReceive %d errno %d %s\n", uMaxReceive, errno, strerror(errno));
				return -1;
			}

		}

		m_vReceiveBuffer.AddDataSize(iReced);


		while (m_vReceiveBuffer.getSize())
		{
			
			switch (m_state)
			{
				case ReadPacketLen:
				{
					if (m_vReceiveBuffer.getSize() < sizeof(short))
					{
						WRITE_INFO("CTcpTask::RecvProcess() more  data needed fd=%d\n", GetSocket());
						return 1; // more  data needed
					}

					short iLen = 0;
					m_vReceiveBuffer.Get<short>(iLen);
					m_iTotalPackLen = ntohs(iLen);
					if (m_iTotalPackLen > (400 * 1024 * 1024) || m_iTotalPackLen <= 0)
					{
						WRITE_ERROR("NeedRecv Body Too Long  May Be Error fd=%d\n", GetSocket());
						return -1;
					}
					m_state = ReadPacket;
				
					m_iDataSize = m_iTotalPackLen - sizeof(short);	   //除去 2Byte的 长度   最少
				}

				case ReadPacket:
				{
					if (!PacketFinished())
					{
						WRITE_INFO("CTcpTask::RecvProcess() more  data needed fd=%d\n", GetSocket());
						return 1; // more  data needed
					}

					ProcessOnePacket();						  // 读取结束 处理一个完整的数据包

					m_state = ReadPacketLen;				   //继续处理 剩余的buf
				}

			}

		}

	} while (0);

	return 0;
}

int CTcpTask::SendData()
{         
        int retLen = 0;
        int retValue = 0;
		unsigned int sendLen = (m_pCurSendMsg->GetLen() - m_iCurSendLen);
		WRITE_INFO("Bgein send to Client fd:%d, ip:%s port:%d len:%d\n", m_iSocket, inet_ntoa(m_SrcAddr.sin_addr), ntohs(m_SrcAddr.sin_port), sendLen);
        retLen = send(GetSocket(), m_pCurSendMsg->GetBody()+ m_iCurSendLen, sendLen, MSG_DONTWAIT);
        if (retLen > 0)
        {               
            m_iCurSendLen += retLen;
            if (m_iCurSendLen == m_pCurSendMsg->GetLen()) //全部发送出去
            {
				WRITE_INFO("CTcpTask::SendData Send To Client Ok  fd:%d, len:%d", m_iSocket, sendLen);
				delete m_pCurSendMsg;
				m_pCurSendMsg = NULL;
				m_iCurSendLen = 0;
				retValue = 0;
            }
            else  //发送了一部分
            {                        
               retValue = 1;
            }
        }
		else //<= 0
		{
			if (errno == EAGAIN || errno == EINTR)
            {
				retValue =  1;
            }
            else											  
            {	// 这里发送出错 直接 就 close(fd) 放弃这个连接
				WRITE_ERROR("CTcpTask::SendData Send data len invalid retLen fd=%d %d errno %d %s", GetSocket(), retLen, errno, strerror(errno));
				retValue = -1;
            }
        }

		// [0 , Ok]   [1 , ReTry] [-1 , error]
        return retValue;
}

CLongConTask::CLongConTask(const string &sAddr, CEpoll* epoll, int iTimeOut /* = 2 */):CTcpTask(-1, epoll)
{
	m_toAddr = sAddr;
	m_iTimeOut = iTimeOut;
}

CLongConTask::CLongConTask(int ipSocket, CEpoll* epoll):CTcpTask(ipSocket, epoll)
{
	
}

int CLongConTask::Init()
{
	Connect(m_toAddr, m_iTimeOut);
	CBaseTask::Init();

	return 0;
}

int CLongConTask::Connect(const string &sAddr, int nTimeOut)
{
	if( nTimeOut < 0 )
		return -1;
	sockaddr_un sToAddr;
	memset(&sToAddr, 0, sizeof(sToAddr));

	m_iSocket = socket(AF_UNIX,  SOCK_STREAM, 0);
	sToAddr.sun_family = AF_UNIX;
	strcpy(sToAddr.sun_path, sAddr.c_str());
	m_toLen = sizeof(sToAddr);

	SetNonBlocking(m_iSocket);
	connect(m_iSocket, (sockaddr*)&m_toAddr, m_toLen);

	struct timeval tvSelectWait;
	tvSelectWait.tv_sec = nTimeOut;
	tvSelectWait.tv_usec = 0;

	fd_set setWrite;
	while(1)
	{
		FD_ZERO(&setWrite);
		FD_SET(m_iSocket, &setWrite);
		int nRet = 0;
		if((nRet = select(m_iSocket + 1, NULL, &setWrite , NULL, &tvSelectWait)) < 0)		// socket出错时返回-1，无事件时返回0 
		{
			if(errno == EINTR) 
				continue;
			return -1;
		}
		else 
		{
			if(nRet == 0) 
				return -1;				//超时 
			else 
				return 0;						//sock可写 
		}
	}	
}


int CUdpTask::RecvProcess()
{
	WRITE_INFO("CUdpTask::RecvProcess Recv from Udp Server fd=%d\n", GetSocket());

	int ret = 0;
	while(1)
	{
		m_vReceiveBuffer.Clear();
        m_fromLen = sizeof(m_fromAddr);
		ret = recvfrom(m_iSocket, m_vReceiveBuffer.JumpBuffer(), MAX_CVBUFFER_SIZE, 0, (sockaddr*)&m_fromAddr, &m_fromLen);
		if (ret <= 0)
		{
			if (errno == EAGAIN || errno == EINTR)
			{
				return 0;
			}

            WRITE_ERROR("CUdpTask::RecvProcess() fd=%d errno %d  %s \n ", GetSocket(), errno, strerror(errno));
			return 0;
		}
		else
		{
			m_vReceiveBuffer.AddDataSize(ret);
		}

		WRITE_INFO("CUdpTask::RecvProcess()  from Udp Server  ip:%s port:%d iLen:%d\n", inet_ntoa(m_fromAddr.sin_addr), ntohs(m_fromAddr.sin_port), ret);

		if (PacketFinished())
		{
			WRITE_INFO("CUdpTask::RecvProcess  from Udp Server receive a full packet process it!\n");
			ProcessOnePacket();
		}
	}
	
	return ret;
}

int CUdpTask::SendData()
{

	int retLen = 0;
	int retValue = 0;
	m_iCurSendLen = 0;
	unsigned int sendLen = (m_pCurSendMsg->GetLen() - m_iCurSendLen);
	retLen = sendto(GetSocket(), m_pCurSendMsg->GetBody() + m_iCurSendLen, sendLen, 0, (sockaddr*)&(m_pCurSendMsg->GetAddr()), m_pCurSendMsg->GetAddrLen());
	if (retLen > 0)
	{               
		m_iCurSendLen += retLen;
		if (m_iCurSendLen == m_pCurSendMsg->GetLen()) //全部发送出去
		{          
			WRITE_INFO("CUdpTask::SendData Send To Udp Server  Ok fd:%d ip:%s port:%d \n", GetSocket(), inet_ntoa(m_pCurSendMsg->GetAddr().sin_addr), ntohs(m_pCurSendMsg->GetAddr().sin_port));
			delete m_pCurSendMsg;
			m_pCurSendMsg = NULL;
			m_iCurSendLen = 0;
			retValue = 0;         
		}
		else  //发送了一部分
		{                        
			retValue = 1;
		}
	}
	else //<= 0
	{
		if (errno == EAGAIN || errno == EINTR)
		{
			retValue = 1;
		}
		else											  
		{	// 可能 是 别的错误 比如 地址错误 或者 对方udp 没有打开错误  如果是对方地址没有打开 udp 发送第二次会走到这里，经过测试过的， 这时候直接就放弃本次数据

			WRITE_ERROR(" CUdpTask::SendData Send data len invalid retLen fd=%d %d errno %d %s", GetSocket(), retLen, errno, strerror(errno));
			//retValue = -1;	  // 本来应该 -1 但是 recv fd 和send fd 一样 不能返回 -1  看  CBaseTask::HandleProcess(int events)
			
			delete m_pCurSendMsg;
			m_pCurSendMsg = NULL;

			retValue = 0;
		}
	}

	// [0 , Ok]   [1 , ReTry] [-1 , error]
	return retValue;
}

int CUdpTask::CreateBindSocket()
{
	WRITE_INFO(" CUdpTask::CreateBindSocket \n");

	int reuse_addr = 1;

	memset((char *)&m_BindAddr, 0, sizeof(m_BindAddr));
	m_BindAddr.sin_family = AF_INET;
	m_BindAddr.sin_port = htons(m_iPort);
	//int ret = 0;

	if (m_sAddr != "")
	{
		inet_aton(m_sAddr.c_str(), &m_BindAddr.sin_addr);
	}
	else
	{
		m_BindAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	}


	int iSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (iSocket < 0) 
	{
		WRITE_ERROR("Socket error");
		return -1;
	}

	setsockopt(iSocket, SOL_SOCKET, SO_REUSEADDR, (void*)(&(reuse_addr)), sizeof(reuse_addr));

	SetNonBlocking(iSocket);

	if (::bind(iSocket, (struct sockaddr *) &m_BindAddr, sizeof(m_BindAddr)) < 0)
	{
		WRITE_ERROR("Bind  port %d error, error(errno is %d) is %s", m_iPort, errno, strerror(errno));
		close(iSocket);
		return -1;
	}

    m_iSocket = iSocket;

	if (iSocket)
	{
		WRITE_INFO("CUdpTask::CreateBindSocket Init\n");
		Init();
	}
	
	return 0;
}
int CUdpTask::PacketFinished()
{
	short iLen = 0;
	m_vReceiveBuffer.Get<short>(iLen);
	unsigned short sLen = ntohs(iLen);
	m_iDataSize = sLen - sizeof(unsigned short);
	WRITE_INFO("iNeedRecvLen= %d\n", m_iCurSendLen);
	return (sLen <= m_vReceiveBuffer.getSize() ? 1 : 0);
}

int CUdpTask::ProcessOnePacket()
{
	//'2(headlen)+headmsg+2(datalen)+datamsg
	// 解析数据

	// 发往 client


	WRITE_INFO("CUdpTask::ProcessOnePacket  from ip:%s port:%d\n", inet_ntoa(m_fromAddr.sin_addr), ntohs(m_fromAddr.sin_port));

	CSendMsg* pMsg = new CSendMsg(m_iDataSize, m_vReceiveBuffer.JumpBuffer());		

	CMyserver::GetInstance()->PutMsgToList(pMsg);
	return 0;

}

CAcceptTask::CAcceptTask(const string &sAddr, CEpoll* epoll)
{
	m_iSocket = -1;
	m_pEpoll = epoll;
	m_sAddr = sAddr;
}

int CAcceptTask::CreateListenSocket()
{
	int reuse_addr = 1;
	unlink(m_sAddr.c_str());
	memset((char *)&m_ListenAddr, 0, sizeof(m_ListenAddr));
	m_ListenAddr.sun_family = AF_UNIX;

	int ret = 0;
	strcpy(m_ListenAddr.sun_path, m_sAddr.c_str());

	int iSocket = socket(AF_UNIX, SOCK_STREAM, 0);
	if (iSocket < 0) 
	{
		WRITE_ERROR("Socket error %s", strerror(errno));
		return -1;
	}

	setsockopt(iSocket, SOL_SOCKET, SO_REUSEADDR, (void*)(&(reuse_addr)), sizeof(reuse_addr));

	SetNonBlocking(iSocket);

	if (::bind(iSocket, (struct sockaddr *) &m_ListenAddr, sizeof(m_ListenAddr)) < 0)
	{
		WRITE_ERROR("Bind  error, error(errno is %d) is %s",  errno, strerror(errno));
		close(iSocket);
		return -1;
	}

	                
	ret = listen(iSocket, 250); 
	if (ret == -1)
	{
		return -1;
	}
	
	if (iSocket)
	{
		m_iSocket = iSocket;
		Init();
	}
	
	return 0;
}

int CAcceptTask::HandleProcess(int event)
{

	if (event & EPOLLIN)
	{
		static int accept_num;
		struct sockaddr_un tmpAddr;
		int iSocketSize=sizeof(sockaddr_un);
		//int ret  = 1;
		int iSocket;
		while (1)
		{                                             
			memset(&tmpAddr, 0, sizeof(sockaddr_in));
			iSocket = accept(GetSocket(), (struct sockaddr *)&tmpAddr, (socklen_t*)&iSocketSize);
			if (iSocket > 0)
			{
				if(accept_num > 10 * 1024 * 1024)
				{
					WRITE_ERROR("too much fd \n");
					close(iSocket);
					continue;
				}

                CBaseTask::SetNonBlocking(iSocket);
	
				CLongConTask *pTcpTask = new CLongConTask(iSocket, m_pEpoll);

				if (pTcpTask != NULL)
				{
					pTcpTask->Init();
					//pTcpTask->SetAddr(tmpAddr);
					{
    						std::string ip(tmpAddr.sun_path);
						WRITE_INFO("CAcceptTask::HandleProcess \t Accept a   New   Client  fd=%d\n", iSocket);
					}

					CMySingleton<CLongConTask>::SetInstance(pTcpTask);
				}
				else
				{
					close(iSocket);
				}
				
			}
			else  //句柄accept完毕
			{
				break;
			}
		}
	}
	else
	{
		WRITE_ERROR("Receive invalid event%d", event);
	}
	return 0;
}
