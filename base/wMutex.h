
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_MUTEX_H_
#define _W_MUTEX_H_

#include <pthread.h>

#include "wNoncopyable.h"
#include "wCond.h"

class wMutex : private wNoncopyable
{
	friend class wCond;
	public:
		wMutex(int kind = PTHREAD_MUTEX_FAST_NP) 
		{
			pthread_mutexattr_t attr;
			pthread_mutexattr_init(&attr);
			pthread_mutexattr_settype(&attr, kind);
			pthread_mutex_init(&mMutex, &attr);
		}
		
		~wMutex()
		{
			pthread_mutex_destroy(&mMutex);
		}
		
		inline void Lock()
		{
			pthread_mutex_lock(&mMutex);
		}
		
		inline void Unlock()
		{
			pthread_mutex_unlock(&mMutex);
		}
		
	protected:
		pthread_mutex_t mMutex;
};


#endif