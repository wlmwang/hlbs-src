
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
		wCond()
		{
			pthread_cond_init(&mCond, NULL);
		}
		
		~zCond()
		{
			pthread_cond_destroy(&mCond);
		}
		
		void Broadcast()
		{
			pthread_cond_broadcast(&mCond);
		}
		
		void Signal()
		{
			pthread_cond_signal(&mCond);
		}

		/**
		 * 等待特定的条件变量满足
		 * @param stMutex 需要等待的互斥体
		 */
		void Wait(wMutex &stMutex)
		{
			pthread_cond_wait(&mCond, &stMutex.mMutex);
		}

	private:

		pthread_cond_t mCond;		/**< 系统条件变量 */

};


#endif