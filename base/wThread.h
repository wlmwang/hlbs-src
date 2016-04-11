
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _W_THREAD_H_
#define _W_THREAD_H_

#include <pthread.h>

#include "wCore.h"
#include "wMutex.h"
#include "wNoncopyable.h"

enum PTHREAD_STATUS
{
	THREAD_INIT = 0,
	THREAD_BLOCKED = 1,
	THREAD_RUNNING = 2,
	THREAD_STOPPED = 3
};

void* ThreadProc(void *pvArgs);

class wThread : private wNoncopyable
{
	public:
		wThread();
		virtual ~wThread();

		virtual int PrepareRun() = 0;
		virtual int Run() = 0;
		virtual bool IsBlocked() { return false; }

		int StartThread(int join = 1);
		int StopThread();
		int Wakeup();
		int CancelThread();
		
		bool IsRunning()
		{
			return mRunStatus == THREAD_RUNNING;
		}
		
		bool IsStop()
		{
			return mRunStatus == THREAD_STOPPED;
		}
		
		pthread_t GetTid()
		{
			return mTid;
		}
		
		virtual char* GetRetVal()
		{
			memcpy(mRetVal, "pthread exited", sizeof("pthread exited"));
			return mRetVal;
		}
	protected:
		int CondBlock();

		pthread_t mTid;
		pthread_attr_t mAttr;
		int mRunStatus;
		char mRetVal[255];
		wMutex *mMutex;
		wCond *mCond;
};

#endif
