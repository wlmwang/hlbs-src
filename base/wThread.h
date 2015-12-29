
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_THREAD_H_
#define _W_THREAD_H_

/**
 *  实现了一个线程的父类
 */

#include <pthread.h>
#include "wNoncopyable.h"

enum eRunStatus
{
	rt_init = 0,
	rt_blocked = 1,
	rt_running = 2,
	rt_stopped = 3
};

void* ThreadProc(void *pvArgs);

class wThread : private wNoncopyable
{
	public:
		wThread();
		virtual ~wThread();

		virtual int PrepareRun() = 0;
		virtual int Run() = 0;
		virtual bool IsBlocked() = 0;

		int CreateThread();
		int StopThread();
		int WakeUp();
		
		bool IsRunning()
		{
			return mRunStatus == rt_running;
		}
		bool IsStop()
		{
			return mRunStatus == rt_stopped;
		}
	protected:
		int CondBlock();

		pthread_t mPhreadId;
		pthread_attr_t mPthreadAttr;
		pthread_mutex_t mMutex;
		pthread_cond_t mCond;
		int mRunStatus;
		char mAbyRetVal[64];
};

#endif
