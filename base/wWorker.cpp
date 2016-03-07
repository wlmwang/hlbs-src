
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include "wWorker.h"

wWorker::wWorker(int iSlot) 
{
	Initialize();
}

wWorker::~wWorker() {}

void wWorker::PrepareRun() {}

void wWorker::Run() {}

int wWorker::InitChannel()
{
	return mCh.Open();
}

void wWorker::Close()
{
	mCh.Close();
}

void wWorker::Initialize() 
{
	mProcess = PROCESS_WORKER;
	mStatus = WORKER_INIT;
	mPid = -1;	//此时worker还未生成
	mUid = 0;
	mGid = 0;
	mPriority = 0;
	mRlimitCore = 1024;
	memcpy(mWorkingDir, PREFIX, strlen(PREFIX)+1);

	mDetached = 0;
	mExited = 0;
	mExiting = 0;
	mRespawn = PROCESS_NORESPAWN;
	mData = NULL;

	mSlot = iSlot;
	mWorkerNum = 0;
	mWorkerPool = NULL;
	mUseMutex = 0;
	mShmAddr = NULL;
	mMutex = NULL;
	mMutexHeld = 0;
}

void wWorker::InitWorker(int iWorkerNum, wWorker **pWorkerPool, int iUseMutex, wShm *pShmAddr, wShmtx *pMutex, int iDelay) 
{
	mWorkerNum = iWorkerNum;
	mWorkerPool = pWorkerPool;
	mUseMutex = iUseMutex;
	mShmAddr = pShmAddr;
	mMutex = pMutex;
	mDelay = iDelay;
}

void wWorker::PrepareStart(int iType, void *pData) 
{
	//mSlot = *(int *) pData;
	mData = pData;
	mRespawn = iType;
	mPid = getpid();
	
	/**
	 *  设置当前进程优先级。进程默认优先级为0
	 *  -20 -> 20 高 -> 低。只有root可提高优先级，即可减少priority值
	 */
	if(mSlot >= 0 && mPriority != 0)
	{
        if (setpriority(PRIO_PROCESS, 0, mPriority) == -1) 
		{
			LOG_ERROR(ELOG_KEY, "[runtime] setpriority(%d) failed: %s", mPriority, strerror(errno));
        }
    }
	
	/**
	 *  设置进程的最大文件描述符
	 */
    if(mRlimitCore != -1) 
	{
        rlmt.rlim_cur = (rlim_t) mRlimitCore;
        rlmt.rlim_max = (rlim_t) mRlimitCore;
        if (setrlimit(RLIMIT_NOFILE, &rlmt) == -1) 
		{
			LOG_ERROR(ELOG_KEY, "[runtime] setrlimit(RLIMIT_NOFILE, %i) failed: %s", mRlimitCore, strerror(errno));
        }
    }
	
    /**
     * 获取进程的有效UID
     * 若是以root身份运行，则将worker进程降级, 默认是nobody。
     */
	/*
    if (geteuid() == 0) 
	{
        if (setgid(GROUP) == -1) 
		{
			LOG_ERROR(ELOG_KEY, "[runtime] setgid(%d) failed: %s", GROUP, strerror(errno));
            exit(2);
        }

        //附加组ID
        if (initgroups(USER, GROUP) == -1) 
		{
			LOG_ERROR(ELOG_KEY, "[runtime] initgroups(%s, %d) failed: %s", USER, GROUP, strerror(errno));
        }

        //用户ID
        if (setuid(USER) == -1) 
		{
			LOG_ERROR(ELOG_KEY, "[runtime] setuid(%d) failed: %s", USER, strerror(errno));            
			exit(2);
        }
    }
	*/
	
    //切换工作目录
    if (strlen(mWorkingDir) > 0) 
	{
        if (chdir((char *)mWorkingDir) == -1) 
		{
			LOG_ERROR(ELOG_KEY, "[runtime] chdir(\"%s\") failed: %s", mWorkingDir, strerror(errno));                        
			exit(2);
        }
    }
	
	srandom((mPid << 16) ^ time(NULL));  //设置种子值，进程ID+时间
	
	//将其他进程的channel[1]关闭，自己的除外
    for(int n = 0; n < mWorkerNum; n++) 
    {
        if(n == mSlot ||mWorkerPool[n]->mPid == -1|| mWorkerPool[n]->mCh[1] == FD_UNKNOWN) 
        {
            continue;
        }

        if (close(mWorkerPool[n]->mCh[1]) == -1) 
        {
            LOG_ERROR(ELOG_KEY, "[runtime] close() channel failed: %s", strerror(errno));
        }
    }

    //关闭该进程worker进程的ch[0]描述符
    if (close(mWorkerPool[mSlot]->mCh[0]) == -1) 
    {
        LOG_ERROR(ELOG_KEY, "[runtime] close() channel failed: %s", strerror(errno));
    }
	
	//worker进程中不阻塞所有信号
	wSigSet mSigSet;
	if(mSigSet.Procmask(SIG_SETMASK))
	{
		LOG_ERROR(ELOG_KEY, "[runtime] sigprocmask() failed: %s", strerror(errno));
	}

	PrepareRun();
}

void wWorker::Start(bool bDaemon) 
{
	mStatus = WORKER_RUNNING;
	Run();
}
