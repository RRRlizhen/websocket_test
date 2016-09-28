/************************************************************************/
/*                                
 filename	: threadmutex.cpp
 auther		: lizhengping
 date		: 2016-07-21
 compare	: g++
*/
/************************************************************************/


#include "threadmutex.h"

CThreadMutex::CThreadMutex(bool bMutil/* =false */)
{
	if (!bMutil)
	{
		pthread_mutex_init(&m_threadMutex, NULL);
	}
	else
	{
		pthread_mutexattr_t attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
		pthread_mutex_init(&m_threadMutex, &attr);
	}
}

CThreadMutex::~CThreadMutex()
{  
        pthread_mutex_destroy(&m_threadMutex);
}

void CThreadMutex::Lock()
{
        pthread_mutex_lock(&m_threadMutex);
}

void CThreadMutex::UnLock()
{
        pthread_mutex_unlock(&m_threadMutex);
}


