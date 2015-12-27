
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

/*
typedef struct
{
	char szThreadKey[32];
	char szLogBaseName[200];
	long lMaxLogSize;
	int iMaxLogNum;
} TLogCfg;
*/

void* ThreadProc(void *pvArgs);

class wThread : private wNoncopyable
{
	public:
		wThread();
		virtual ~wThread();

		virtual int PrepareRun() = 0;
		virtual int Run() = 0;
		virtual int IsToBeBlocked() = 0;

		int CreateThread();
		int StopThread();
		int WakeUp();
		
		/*
		void ThreadLogInit(char *sPLogBaseName, long lPMaxLogSize, int iPMaxLogNum, int iShow, int iLevel = 0);
		void ThreadLogDebug(const char *sFormat, ...);
		void ThreadLogInfo(const char *sFormat, ...);
		void ThreadLogNotice(const char *sFormat, ...);
		void ThreadLogWarn(const char *sFormat, ...);
		void ThreadLogError(const char *sFormat, ...);
		void ThreadLogFatal(const char *sFormat, ...);
		*/
		
	protected:
		int CondBlock();

		pthread_t mPhreadId;
		pthread_attr_t mPthreadAttr;
		pthread_mutex_t mMutex;
		pthread_cond_t mCond;
		int mRunStatus;
		char mAbyRetVal[64];

		//TLogCfg m_stLogCfg;

	//private:
};

#endif
