/*
 * locks.h
 *
 *  Created on: Jan 2, 2012
 *      Author: sa
 */
#include <pthread.h>
#include <semaphore.h>

#ifndef LOCKS_H_
#define LOCKS_H_


class MyMutex {
private:
	friend class MyCond;
	pthread_mutex_t mutex;
public:
	MyMutex();
	~MyMutex();
	int Lock();
	int TryLock();
	int Unlock();
};
class MyCond
{
	pthread_cond_t data;
public:
	MyCond();
	~MyCond();
	int Wait(MyMutex *mutex);
	int TimedWait(MyMutex *mutex,timespec *time);
	int Signal();
	int Broadcast();
};
class AutoMyMutexLock
{
private:
	MyMutex* pcs;
public:
	AutoMyMutexLock(MyMutex &srpcs):pcs(&srpcs)
	{
		pcs->Lock();
	}
	~AutoMyMutexLock()
	{
		pcs->Unlock();
	}
};
class AutoMyMutexTryLock
{
private:
	MyMutex* pcs;
	bool Locked;
public:
	AutoMyMutexTryLock(MyMutex &srpcs):pcs(&srpcs)
	{
		Locked=pcs->TryLock()==0;
	}
	~AutoMyMutexTryLock()
	{
		if(Locked)
			pcs->Unlock();
	}
	bool IsLocked(){return Locked;}
	operator bool(){return Locked;}
};
class MySem
{
private:
	sem_t data;
public:
	MySem();
	~MySem();
	int Wait();
	int TimedWait(timespec *time);
	int TryWait();
	int Post();
	int GetValue(int *res);
};
class InterlockCounter
{
private:
	long m;
public:
	InterlockCounter():m(0)
	{
	}
	InterlockCounter(const long b)
	{
		m=b;
	}
	operator unsigned long(){return m;}
	long operator++(int)
	{
		return __sync_fetch_and_add(&m,1);
	}
	long operator--(int)
	{
		return __sync_fetch_and_sub(&m,1);
	}
	long operator++()
	{
		return __sync_add_and_fetch(&m,1);
	}
	long operator--()
	{
		return __sync_sub_and_fetch(&m,1);
	}
};
#endif /* LOCKS_H_ */
