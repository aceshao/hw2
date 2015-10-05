#include "thread.h"
#include<pthread.h>

Thread::Thread(void* (*threadFuction)(void*),void* threadArgv) 
{
	pthread_attr_t threadAttr;
	pthread_attr_init(&threadAttr);
	pthread_create(&m_thread, &threadAttr, threadFuction, threadArgv);
}

Thread::~Thread() 
{
	// TODO Auto-generated destructor stub
}

void Thread::JoinThread()
{
	pthread_join(m_thread, NULL);
}

Mutex::Mutex()
{
	pthread_mutex_init(&m_mutex, NULL);
}
Mutex::~Mutex()
{
	//pthread_mutex_destory(&m_mutex);
}
int Mutex::Lock()
{
	return  pthread_mutex_lock(&m_mutex);
}

int Mutex::Unlock()
{
	return pthread_mutex_unlock(&m_mutex);
}

Sem::Sem(int shared, int value)
{
	 sem_init(&m_sem, shared, value);
}

Sem::~Sem()
{
	sem_destroy(&m_sem);
}

int Sem::Wait()
{
	return sem_wait(&m_sem);
}

int Sem::Post()
{
	return sem_post(&m_sem);
}

int Sem::Value()
{
	int value = 0;
	sem_getvalue(&m_sem,&value);
	return value;
}
