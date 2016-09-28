#include "myepoll.h"
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <sys/epoll.h>
#include "basetask.h"
#include "writeRunInfo.h"

CEpoll::CEpoll()
{
	m_nEpollFd = 0;
	m_nEpollSize = 0;
	m_epollEvents = NULL;
}

CEpoll::~CEpoll()
{
	if (m_epollEvents != NULL)
	{
		delete[] m_epollEvents;
	}

	close(m_nEpollFd);
}

int CEpoll::Init(int epollSize, int epollWaitTime)
{
	m_nEpollWaitTime = epollWaitTime;
	m_nEpollSize = epollSize;
	m_nEpollFd = epoll_create(m_nEpollSize);
	if (m_nEpollFd == -1)
	{
		WRITE_ERROR("Epoll init failed, errno is %d, and error is %s", errno, strerror(errno));
		return -1;
	}
	m_epollEvents = new epoll_event[m_nEpollSize];
	return 0;
}

int CEpoll::AddToEpoll(CBaseTask *pTask, int event)
{
	int tmpOprate = EPOLL_CTL_ADD;
	struct epoll_event tmpEvent;
	memset(&tmpEvent, 0, sizeof(epoll_event));
	tmpEvent.events = event;
	tmpEvent.data.ptr = pTask;
	epoll_ctl(m_nEpollFd, tmpOprate, pTask->GetSocket(), &tmpEvent);
	return 0;
}

int CEpoll::DelFromEpoll(CBaseTask *pTask)
{
	int tmpOprate = EPOLL_CTL_DEL;
	struct epoll_event tmpEvent;
	memset(&tmpEvent, 0, sizeof(epoll_event));
	tmpEvent.data.ptr = pTask;
	epoll_ctl(m_nEpollFd, tmpOprate, pTask->GetSocket(), &tmpEvent);
	return 0;
}

int CEpoll::ModifyFromEpoll(CBaseTask *pTask, int event)
{
    int tmpOprate = EPOLL_CTL_MOD;
    struct epoll_event tmpEvent;
    tmpEvent.events = event;  
    tmpEvent.data.ptr = pTask;
    epoll_ctl(m_nEpollFd, tmpOprate, pTask->GetSocket(), &tmpEvent);
    return 0;
}

int CEpoll::EpollWait()
{
    int  nfds =  epoll_wait(m_nEpollFd, m_epollEvents, m_nEpollSize,  m_nEpollWaitTime);
    if (nfds == -1)
    {
            WRITE_ERROR("epoll_wait error(%d) is %s\n", errno, strerror(errno));      
            return 0;
    }      

    for (int i =0; i < nfds; i++)
    {
            CBaseTask * tmpTask = (CBaseTask*)(m_epollEvents[i].data.ptr);
            if (tmpTask != NULL)
            {
                    tmpTask->HandleProcess(m_epollEvents[i].events);
            }
    }

    return nfds;
}




