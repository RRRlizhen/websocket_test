#ifndef __WORK_THREAD_H__
#define __WORK_THREAD_H__

#include "synqueue_t.h"
#include "common.h"
#include <sys/types.h>
#include <pthread.h>

class CWorkThread
{
public:
	CWorkThread();
	virtual ~CWorkThread();

	int Run();

	int Start();

	int PutMsgToInputList(CSendMsg * pMsg);

	virtual int Process(CSendMsg* pMsg);

	void Stop()
	{
		m_nRunFlag = 0;	
	}


protected:
	static void *ThreadEntry(void *arg);
	pthread_t  m_nThreadId; //线程id
	int m_nRunFlag; //0停止1运行 

	CSynQueue<CSendMsg> m_pMsgList;
};


#endif
