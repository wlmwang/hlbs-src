
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "wWorker.h"

wWorker::wWorker(int iSlot) : mSlot(iSlot)
{
#ifdef PREFIX
	memcpy(mWorkingDir, PREFIX, strlen(PREFIX) + 1);
#else
	if (GetCwd(mWorkingDir, sizeof(mWorkingDir)) == -1)
	{
		mErr = errno;
		LOG_ERROR(ELOG_KEY, "[system] getcwd failed: %s", strerror(mErr));
		exit(2);
	}
#endif
	//worker通信 主要通过channel同步个fd，填充进程表
	mIpc = new wWorkerIpc(this);
}

int wWorker::OpenChannel()
{
	return mCh.Open();
}

void wWorker::Close()
{
	mCh.Close();
}

void wWorker::PrepareStart(int iSlot, int iType, string sTitle, void *pData) 
{
	mPid = getpid();
	mSlot = iSlot;
	mRespawn = iType;
	mData = sTitle;
	mMaster = (wMaster<wMaster> *)pData;

	/**
	 *  设置当前进程优先级。进程默认优先级为0
	 *  -20 -> 20 高 -> 低。只有root可提高优先级，即可减少priority值
	 */
	if (mSlot >= 0 && mPriority != 0)
	{
        if (setpriority(PRIO_PROCESS, 0, mPriority) == -1) 
		{
			mErr = errno;
			LOG_ERROR(ELOG_KEY, "[system] setpriority(%d) failed: %s", mPriority, strerror(mErr));
        }
    }
	
	/**
	 *  设置进程的最大文件描述符
	 */
    if (mRlimitCore != -1) 
	{
		struct rlimit rlmt;
        rlmt.rlim_cur = (rlim_t) mRlimitCore;
        rlmt.rlim_max = (rlim_t) mRlimitCore;
        if (setrlimit(RLIMIT_NOFILE, &rlmt) == -1) 
		{
			mErr = errno;
			LOG_ERROR(ELOG_KEY, "[system] setrlimit(RLIMIT_NOFILE, %i) failed: %s", mRlimitCore, strerror(mErr));
        }
    }
	
    //切换工作目录
    if (strlen(mWorkingDir) > 0) 
	{
		LOG_DEBUG(ELOG_KEY, "[system] chdir(\"%s\")", mWorkingDir);
        if (chdir((char *)mWorkingDir) == -1) 
		{
			mErr = errno;
			LOG_ERROR(ELOG_KEY, "[system] chdir(\"%s\") failed: %s", mWorkingDir, strerror(mErr));
			exit(2);
        }
    }
	
	srandom((mPid << 16) ^ time(NULL));  //设置种子值，进程ID+时间
	
	//将其他进程的channel[1]关闭，自己的除外
    for (int n = 0; n < mWorkerNum; n++) 
    {
        if (n == mSlot ||mWorkerPool[n]->mPid == -1|| mWorkerPool[n]->mCh[1] == FD_UNKNOWN) 
        {
            continue;
        }

        if (close(mWorkerPool[n]->mCh[1]) == -1) 
        {
        	mErr = errno;
            LOG_ERROR(ELOG_KEY, "[system] close() channel failed: %s", strerror(mErr));
        }
    }

    //关闭该进程worker进程的ch[0]描述符
    if (close(mWorkerPool[mSlot]->mCh[0]) == -1) 
    {
    	mErr = errno;
        LOG_ERROR(ELOG_KEY, "[system] close() channel failed: %s", strerror(mErr));
    }
	
	//worker进程中不阻塞所有信号
	wSigSet mSigSet;
	if (mSigSet.Procmask(SIG_SETMASK))
	{
		mErr = errno;
		LOG_ERROR(ELOG_KEY, "[system] sigprocmask() failed: %s", strerror(mErr));
	}
	
	PrepareRun();
}

void wWorker::Start(bool bDaemon) 
{
	mStatus = WORKER_RUNNING;
	
	mIpc->StartThread();
	Run();
}
