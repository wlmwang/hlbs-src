
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

int wThread::StartThread()
{
	pthread_attr_init(&mPthreadAttr);
	pthread_attr_setscope(&mPthreadAttr, PTHREAD_SCOPE_SYSTEM);  //设置线程状态为与系统中所有线程一起竞争CPU时间
	pthread_attr_setdetachstate(&mPthreadAttr, PTHREAD_CREATE_JOINABLE);  //设置非分离的线程
	
	mMutex = new wMutex(NULL);
	mCond = new wCond(NULL);

	mRunStatus = rt_running;

	pthread_create(&mPhreadId, &mPthreadAttr, ThreadProc, (void *)this);

	return 0;
}

int wThread::CondBlock()
{
	mMutex->Lock();

	while(IsBlocked() || mRunStatus == rt_stopped)  //线程被阻塞或者停止
	{
		if(mRunStatus == rt_stopped)  //如果线程需要停止则终止线程
		{
			pthread_exit((void *)mAbyRetVal);	//"Thread exit"
		}
		
		mRunStatus = rt_blocked;	//"Thread would blocked"
		
		mCond->Wait(*mMutex);	//进入休眠状态
	}

	if(mRunStatus != rt_running)  
	{
		//"Thread waked up"
	}
	
	mRunStatus = rt_running;  //线程状态变为rt_running

	mMutex->Unlock();	//该过程需要在线程锁内完成
	return 0;
}

int wThread::WakeUp()
{
	mMutex->Lock();

	if(!IsBlocked() && mRunStatus == rt_blocked)
    {
		mCond->Signal();	//向线程发出信号以唤醒
	}

	mMutex->Unlock();
	return 0;
}

int wThread::StopThread()
{
	mMutex->Lock();

	mRunStatus = rt_stopped;
	mCond->Signal();

	mMutex->Unlock();
	
	//等待该线程终止
	pthread_join(mPhreadId, NULL);

	return 0;
}