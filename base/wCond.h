
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_COND_H_
#define _W_COND_H_

#include <pthread.h>

#include "wNoncopyable.h"
#include "wMutex.h"

class wCond : private wNoncopyable
{
	public:
		wCond(int pshared = PTHREAD_PROCESS_PRIVATE)
		{
			pthread_condattr_init(&mAttr);
			pthread_condattr_setpshared(&mAttr, pshared);
			pthread_cond_init(&mCond, &mAttr);
		}
		
		~wCond()
		{
			pthread_condattr_destroy(&mAttr);
			pthread_cond_destroy(&mCond);
		}
		
		int Broadcast()
		{
			return pthread_cond_broadcast(&mCond);
		}
		
		/**
		 *  使用pthread_cond_signal不会有“惊群现象”产生，他最多只给一个线程发信号
		 */
		int Signal()
		{
			return pthread_cond_signal(&mCond);
		}

		/**
		 * 等待特定的条件变量满足
		 * @param stMutex 需要等待的互斥体
		 */
		int Wait(wMutex &stMutex)
		{
			return pthread_cond_wait(&mCond, &stMutex.mMutex);
		}
		
		int TimeWait(wMutex &stMutex, struct timespec *tsptr)
		{
			return pthread_cond_timewait(&mCond, &stMutex.mMutex, tsptr);
		}
		
	private:
		pthread_cond_t mCond;		//系统条件变量
		pthread_condattr_t mAttr;
};


#endif