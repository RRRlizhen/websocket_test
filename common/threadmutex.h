/************************************************************************/
/*                                
 filename	: threadmutex.h
 auther		: lizhengping
 date		: 2016-07-21
 compare	: g++
*/
/************************************************************************/

#ifndef  __THREAD_MUTEX_H
#define __THREAD_MUTEX_H
#include <pthread.h>
#include <sys/time.h>
#include <errno.h>

class CThreadCond;
class CThreadMutex
{
friend class CThreadCond;
public:
         CThreadMutex(const bool bMutil=false);
         ~CThreadMutex();
         void Lock();
         void UnLock();
private:
        pthread_mutex_t  m_threadMutex;
};

class CThreadLock
{
public:
        CThreadLock(CThreadMutex *pMutex)
        {
            m_pMutex = pMutex;
            m_pMutex->Lock();
        }
        ~CThreadLock()
        {
            m_pMutex->UnLock();
        }
private:
        CThreadMutex *m_pMutex;
};


class CThreadCond
{
public:
	CThreadCond()
	{
		_is_init = false;
		_is_create = false;
	}
	~CThreadCond()
	{
		
	}

	CThreadCond(const CThreadCond &cond)
	{
		_is_create = false;
		_is_init = cond._is_init;
		_mutex = cond._mutex;
		_cond = cond._cond;
	}

	void init()
	{
		pthread_mutex_init(&_mutex, NULL);
		pthread_cond_init(&_cond, NULL);		
		_is_create = true;
		_is_init = true;
	}

	void destory()
	{
		if (_is_create)
		{
			pthread_mutex_destroy(&_mutex);
			pthread_cond_destroy(&_cond);
			_is_init = false;
		}
	}

	int WaitCondTimeOut(int msec)//等待的时间是毫秒
	{
		if (!_is_init)
			return 2;
		
		timeval tm;
		gettimeofday(&tm, NULL);
		unsigned long long tmp = tm.tv_sec * 1000000 + tm.tv_usec + msec * 1000;
		timespec to;
		to.tv_sec = tmp / 1000000;
		to.tv_nsec = (tmp - to.tv_sec * 1000000)*1000;
		pthread_mutex_lock(&_mutex);
		int ret = pthread_cond_timedwait(&_cond, &_mutex, &to);
		pthread_mutex_unlock(&_mutex);
		return ret;
	}

	int WaitCond()
	{	
		if (!_is_init)
			return 2;
		pthread_mutex_lock(&_mutex);
		int ret = pthread_cond_wait(&_cond, &_mutex);
		pthread_mutex_unlock(&_mutex);
		return ret;
	}

	int SetCond()
	{
		if (!_is_init)
			return 2;
		pthread_mutex_lock(&_mutex);
		int ret = pthread_cond_signal(&_cond);
		pthread_mutex_unlock(&_mutex);
		return ret;
	}
private:
	bool _is_init;
	bool _is_create;
	pthread_mutex_t _mutex;
	pthread_cond_t _cond; 
};

#endif

