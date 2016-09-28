
#ifndef __SYN_QUEUE_H__
#define __SYN_QUEUE_H__
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>	 
#include <deque>	 

using namespace std;
	 
#define MAX_QUEUE_SIZE	512000
#define CQUE_PRI_HIGH   1

template<class T>	 
class CSynQueue
{
public:
	 CSynQueue()
	 {
			 m_maxsize = MAX_QUEUE_SIZE;
			 sem_init(&m_sem_write, 0, m_maxsize - 1);
			 sem_init(&m_sem_read,	0, 0);
			 pthread_mutex_init(&m_mutex_write, NULL);
	 }
	 ~CSynQueue()
	 {
			 sem_destroy(&m_sem_read);
			 sem_destroy(&m_sem_write);
			 pthread_mutex_destroy(&m_mutex_write);
	 }

	 int  put(T *pMsg)
	 {
			 sem_wait(&m_sem_write);

			 pthread_mutex_lock(&m_mutex_write);
			 m_queue.push_back(pMsg);
			 pthread_mutex_unlock(&m_mutex_write);

			 return sem_post(&m_sem_read);
	 }
 
	 int  put(T *pMsg, int pri)
	 {
			 sem_wait(&m_sem_write);

			 pthread_mutex_lock(&m_mutex_write);
			 if (CQUE_PRI_HIGH == pri)
			 {
			 m_queue.push_front(pMsg); 
			 }
			 pthread_mutex_unlock(&m_mutex_write);

			 return sem_post(&m_sem_read);
	 }
 
	 T*	 get()
	 {	  
			 T *pMsg = NULL;
			 sem_wait(&m_sem_read);    
			 pthread_mutex_lock(&m_mutex_write);
			 DequeueIterator tmpItr = m_queue.begin();
			 if (tmpItr != m_queue.end())
			 {
					 pMsg  = *tmpItr;
					 m_queue.pop_front();
			 }	
			 pthread_mutex_unlock(&m_mutex_write);
			 sem_post(&m_sem_write);
			 return pMsg;
	 }

	 size_t  size()
	 {
			 return m_queue.size();
	 }

	 size_t  length()
	 {
			 return m_maxsize;
	 }

private:
	size_t m_maxsize;
	pthread_mutex_t m_mutex_write;
	sem_t m_sem_read;				 /* 可读取的元素个数 */
	sem_t m_sem_write; 			 /* 可写入的空位个数 */ 	
	deque<T*> m_queue;				 /* 保存数据的队列 */
	typedef typename deque<T*>::iterator DequeueIterator;
};	 
#endif 


