
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include "wWorker.h"

wWorker::wWorker() 
{
	Initialize();
}

wWorker::~wWorker() {}

void wWorker::Initialize(int iWorkerNum, wWorker **pWorkerPool, int iUseMutex, wShm *pShmAddr, wShmtx *pMutex) 
{
	mPid = -1;
	mSlot = -1;
	mExited = -1;
	mRespawn = 0;
	mWorkerNum = 0;
	mWorkerPool = NULL;
	mUseMutex = 0;
	mShmAddr = NULL;
	mMutex = NULL;

	if (iWorkerNum > 0)
	{
		mWorkerNum = iWorkerNum;
	}
	if (pWorkerPool != 0)
	{
		mWorkerPool = pWorkerPool;
	}
	if (iUseMutex > 0)
	{
		mUseMutex = iUseMutex;
	}
	if (pShmAddr != 0)
	{
		mShmAddr = pShmAddr;
	}
	if (pMutex != 0)
	{
		mMutex = pMutex;
	}
}

int wWorker::InitChannel()
{
	return mCh.Open();
}

void wWorker::Close()
{
	mCh.Close();
}

void wWorker::PrepareRun() {}

void wWorker::Run() {}

void wWorker::PrepareStart(int type, void *data) 
{
	mPid = getpid();
	mSlot = *(int *) data;
	mRespawn = type;
	mExited = 0;
	
	//不阻塞所有信号
	wSigSet mSigSet;
	if(mSigSet.Procmask(SIG_SETMASK))
	{
		LOG_ERROR(ELOG_KEY, "[runtime] sigprocmask() failed: %s", strerror(errno));
	}
	
	//将其他进程的channel[1]关闭，自己的除外
    for (int n = 0; n < mWorkerNum; n++) 
    {
        if (mWorkerPool[n]->mPid == -1) 
        {
            continue;
        }
        if (n == mSlot) 
        {
            continue;
        }
        if (mWorkerPool[n]->mCh[1] == -1) 
        {
            continue;
        }

        if (close(mWorkerPool[n].mCh[1]) == -1) 
        {
            LOG_ERROR(ELOG_KEY, "[runtime] close() channel failed: %s", strerror(errno));
        }
    }

    //关闭属于该worker进程的channel[0]套接字描述符。这个描述符是由master进程所使用的
    if (close(mWorkerPool[mSlot].mCh[0]) == -1) 
    {
        LOG_ERROR(ELOG_KEY, "[runtime] close() channel failed: %s", strerror(errno));
    }

    /*
    if (ngx_add_channel_event(cycle, ngx_channel, NGX_READ_EVENT,ngx_channel_handler) == NGX_ERROR)
    {
        exit(2);
    }
    */
   
	PrepareRun();
}

void wWorker::Start() 
{
	Run();
	
	//进程退出
	exit(0);
}

