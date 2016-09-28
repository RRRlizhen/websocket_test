#ifndef  __MYEPOLL_H__
#define  __MYEPOLL_H__
#include "sys/epoll.h" 
#include "errno.h"

#define   EPOLLSIZE			10* 1024 * 1024
#define   EPOLLWAITTIME		10
class CBaseTask;
class CEpoll
{
public:
     CEpoll();

     int Init(int epollSize=EPOLLSIZE, int epollWaitTime=EPOLLWAITTIME);
     
	 ~CEpoll();
     
     int AddToEpoll(CBaseTask *pTask, int event);
     
     int DelFromEpoll(CBaseTask *pTask);    
     
     int ModifyFromEpoll(CBaseTask *pTask, int event);    
     
     int EpollWait();

private:
     int  m_nEpollFd;
     int m_nEpollSize;
     int m_nEpollWaitTime;
     struct epoll_event *m_epollEvents;
};

#endif
