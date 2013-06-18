/*
 * locks.cpp
 *
 *  Created on: Jan 2, 2012
 *      Author: sa
 */

#include "locks.h"

MyMutex::MyMutex() {
	pthread_mutexattr_t mutexattr;
	pthread_mutexattr_init(&mutexattr);
	pthread_mutexattr_setpshared(&mutexattr, PTHREAD_PROCESS_PRIVATE);
	pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&mutex, &mutexattr);
	pthread_mutexattr_destroy(&mutexattr);
}
MyMutex::~MyMutex() {
	pthread_mutex_destroy(&mutex);
}
int MyMutex::Lock() {
	return pthread_mutex_lock(&mutex);
}
int MyMutex::TryLock() {
	return pthread_mutex_trylock(&mutex);
}
int MyMutex::Unlock() {
	return pthread_mutex_unlock(&mutex);
}
MyCond::MyCond() {
	pthread_condattr_t attr;
	pthread_condattr_init(&attr);
	pthread_condattr_setpshared(&attr, PTHREAD_PROCESS_PRIVATE);
	pthread_condattr_setclock(&attr, 0);
	pthread_cond_init(&data, &attr);
	pthread_condattr_destroy(&attr);
}
MyCond::~MyCond() {
	pthread_cond_destroy(&data);
}
int MyCond::Wait(MyMutex *mutex) {
	return pthread_cond_wait(&data, &(mutex->mutex));
}
int MyCond::TimedWait(MyMutex *mutex, timespec *time) {
	return pthread_cond_timedwait(&data, &(mutex->mutex), time);
}
int MyCond::Signal() {
	return pthread_cond_signal(&data);
}
int MyCond::Broadcast() {
	return pthread_cond_broadcast(&data);
}

MySem::MySem() {
	sem_init(&data, PTHREAD_PROCESS_PRIVATE, 0);
}
MySem::~MySem() {
	sem_destroy(&data);
}
int MySem::Wait() {
	return sem_wait(&data);
}
int MySem::TimedWait(timespec *time) {
	return sem_timedwait(&data, time);
}
int MySem::TryWait() {
	return sem_trywait(&data);
}
int MySem::Post() {
	return sem_post(&data);
}
int MySem::GetValue(int *res) {
	return sem_getvalue(&data, res);
}

