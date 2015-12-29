
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include <stdarg.h>
#include <string.h>

#include "wThread.h"
#include "wLog.h"

void* ThreadProc(void *pvArgs)
{
	if(!pvArgs)
	{
		return NULL;
	}

	wThread *pThread = (wThread *)pvArgs;

	if(pThread->PrepareRun())
	{
		return NULL;
	}

	pThread->Run();

	return NULL;
}

wThread::wThread()
{
	mRunStatus = rt_init;
}

wThread::~wThread() 
{
	//...
}

int wThread::CreateThread()
{
	pthread_attr_init(&mPthreadAttr);
	pthread_attr_setscope(&mPthreadAttr, PTHREAD_SCOPE_SYSTEM);  // 设置线程状态为与系统中所有线程一起竞争CPU时间
	pthread_attr_setdetachstate(&mPthreadAttr, PTHREAD_CREATE_JOINABLE);  // 设置非分离的线程
	
	pthread_cond_init(&mCond, NULL);
	pthread_mutex_init(&mMutex, NULL);
	
	mRunStatus = rt_running;

	pthread_create(&mPhreadId, &mPthreadAttr, ThreadProc, (void *)this);

	return 0;
}

int wThread::CondBlock()
{
	pthread_mutex_lock(&mMutex);

	while(IsBlocked() || mRunStatus == rt_stopped)  // 线程被阻塞或者停止
	{
		if(mRunStatus == rt_stopped)  // 如果线程需要停止则终止线程
		{
			//"Thread exit."
			pthread_exit((void *)mAbyRetVal);
		}
		//"Thread would blocked."
		mRunStatus = rt_blocked;
		pthread_cond_wait(&mCond, &mMutex);  //进入休眠状态
	}

	if(mRunStatus != rt_running)  
	{
		//"Thread waked up."
	}
	
	mRunStatus = rt_running;  //线程状态变为rt_running

	pthread_mutex_unlock(&mMutex);  //该过程需要在线程锁内完成

	return 0;
}

int wThread::WakeUp()
{
	pthread_mutex_lock(&mMutex);

	if(!IsBlocked() && mRunStatus == rt_blocked)
    {
		pthread_cond_signal(&mCond);  //向线程发出信号以唤醒
	}

	pthread_mutex_unlock(&mMutex);

	return 0;
}

int wThread::StopThread()
{
	pthread_mutex_lock(&mMutex);

	mRunStatus = rt_stopped;
	pthread_cond_signal(&mCond);

	pthread_mutex_unlock(&mMutex);

	//等待该线程终止
	pthread_join(mPhreadId, NULL);

	return 0;
}