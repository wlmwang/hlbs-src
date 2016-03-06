
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include "wWorker.h"

wWorker::wWorker(int iSlot) 
{
	Initialize();
	
	mSlot = iSlot;
	mPid = -1;
	mExited = -1;
	mExiting = -1;
	mRespawn = PROCESS_NORESPAWN;
}

wWorker::~wWorker() {}

void wWorker::Initialize(int iWorkerNum, wWorker **pWorkerPool, int iUseMutex, wShm *pShmAddr, wShmtx *pMutex) 
{	
	mWorkerNum = iWorkerNum > 0? iWorkerNum : 0;
	mWorkerPool = pWorkerPool != NULL? pWorkerPool : NULL;
	mUseMutex = iUseMutex > 0? iUseMutex : 0;
	mShmAddr = pShmAddr != NULL ? pShmAddr : NULL;
	mMutex = pMutex != NULL ? pMutex : NULL;
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
	mSlot = *(int *) data;
	mRespawn = type;
	mPid = getpid();
	
	/**
	 *  设置当前进程优先级。进程默认优先级为0
	 *  -20 -> 20 高 -> 低。只有root可提高优先级，即可减少priority值
	 */
	int priority = 0;
	if (mSlot >= 0 && /*priority != 0*/) 
	{
        if (setpriority(PRIO_PROCESS, 0, priority) == -1) 
		{
			LOG_ERROR(ELOG_KEY, "[runtime] setpriority(%d) failed: %s", priority, strerror(errno));
        }
    }
	
	/**
	 *  设置进程的最大文件描述符，内核默认是1024
	 */
	int rlimit_nofile = 65535;
    if (rlimit_nofile != -1) 
	{
        rlmt.rlim_cur = (rlim_t) rlimit_nofile;
        rlmt.rlim_max = (rlim_t) rlimit_nofile;
		
        if (setrlimit(RLIMIT_NOFILE, &rlmt) == -1) 
		{
			LOG_ERROR(ELOG_KEY, "[runtime] setrlimit(RLIMIT_NOFILE, %i) failed: %s", rlimit_nofile, strerror(errno));
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
    if (strlen(PREFIX) > 0) 
	{
        if (chdir((char *) PREFIX) == -1) 
		{
			LOG_ERROR(ELOG_KEY, "[runtime] chdir(\"%s\") failed: %s", PREFIX, strerror(errno));                        
			exit(2);
        }
    }
	
	//worker进程中不阻塞所有信号
	wSigSet mSigSet;
	if(mSigSet.Procmask(SIG_SETMASK))
	{
		LOG_ERROR(ELOG_KEY, "[runtime] sigprocmask() failed: %s", strerror(errno));
	}
	
	srandom((mPid << 16) ^ time(NULL));  //设置种子值，进程ID+时间
	
	//将其他进程的channel[1]关闭，自己的除外
    for (int n = 0; n < mWorkerNum; n++) 
    {
        if (mWorkerPool[n]->mPid == -1|| mWorkerPool[n]->mCh[1] == -1|| n == mSlot) 
        {
            continue;
        }

        if (close(mWorkerPool[n]->mCh[1]) == -1) 
        {
            LOG_ERROR(ELOG_KEY, "[runtime] close() channel failed: %s", strerror(errno));
        }
    }

    //关闭属于该worker进程的channel[0]套接字描述符。这个描述符是由master进程所使用的
    if (close(mWorkerPool[mSlot]->mCh[0]) == -1) 
    {
        LOG_ERROR(ELOG_KEY, "[runtime] close() channel failed: %s", strerror(errno));
    }
   
	PrepareRun();
}

void wWorker::Start(bool bDaemon) 
{
	Run();
	
	//TODO 信号处理
	//
	//进程退出
	exit(0);
}
