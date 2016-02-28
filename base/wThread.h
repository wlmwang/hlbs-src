
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_THREAD_H_
#define _W_THREAD_H_

#include <pthread.h>
#include "wMutex.h"
#include "wCond.h"
#include "wNoncopyable.h"

enum eRunStatus
{
	RT_INIT = 0,
	RT_BLOCKED = 1,
	RT_RUNNING = 2,
	RT_STOPPED = 3
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

		int StartThread(int join = 1);
		int StopThread();
		int Wakeup();
		int CancelThread();
		
		bool IsRunning()
		{
			return mRunStatus == RT_RUNNING;
		}
		
		bool IsStop()
		{
			return mRunStatus == RT_STOPPED;
		}
		
		pthread_t GetTid()
		{
			return mTid;
		}
		
		virtual char* GetRetVal()
		{
			mRetVal = "pthread exited";
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
