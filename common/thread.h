#ifndef THREAD_H
#define THREAD_H

#include <pthread.h>
#include <semaphore.h>

class Thread 
{
private:
	pthread_t m_thread; 
public:
	Thread(void* (*threadFuction)(void*),void* threadArgv);
	virtual ~Thread();
	void JoinThread();
};

class Mutex
{
private:
	pthread_mutex_t m_mutex;
public:
	Mutex();
	virtual ~Mutex();
	int Lock();
	int Unlock();
};

class Sem
{
private:
	sem_t m_sem;
public:
	Sem(int shared,int value);
	virtual ~Sem();
	int Wait();
	int Post();

	int Value();
};
#endif

