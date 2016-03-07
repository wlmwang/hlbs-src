
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_MASTER_H_
#define _W_MASTER_H_

#include <map>
#include <vector>

#include "wType.h"
#include "wLog.h"
#include "wMisc.h"
#include "wSingleton.h"
#include "wFile.h"
#include "wShm.h"
#include "wShmtx.h"
#include "wChannel.h"
#include "wSigSet.h"
#include "wSignal.h"
#include "wWorker.h"
#include "wCoreCmd.h"

template <typename T>
class wMaster : public wSingleton<T>
{
	public:
		wMaster();
		virtual ~wMaster();
		void Initialize();
		
		void PrepareStart();
		void MasterStart();		//master-worker模式启动
		void SingleStart();		//单进程模式启动
		
		void WorkerStart(int n, int type = PROCESS_RESPAWN);
		pid_t SpawnWorker(int i, const char *title, int type = PROCESS_RESPAWN);
		void PassOpenChannel(struct ChannelReqOpen_t* pCh);
		virtual wWorker* NewWorker(int iSlot = 0);
		virtual void HandleSignal();

		/**
		 * master-worker工作模式主要做一下事情：
		 * 1. 设置进程标题 
		 * 2. 设置pid文件名
		 * 3. 设置启动worker个数
		 * 4. 设置自定义信号处理结构
		 * 5. 初始化服务器（创建、bind、listen套接字） 
		 */
		virtual void PrepareRun();
		virtual void Run();
		
		/**
		 *  注册信号回调
		 *  可重写全局变量g_signals，实现自定义信号处理
		 */
		void InitSignals();
		
		int CreatePidFile();
		void DeletePidFile();

	public:
		MASTER_STATUS mStatus;
		int mProcess;
		int mNcpu;		//cpu个数
		pid_t mPid;		//master进程id
		int mSlot;		//进程表分配到数量
		int mWorkerNum;	//worker总数量
		wWorker **mWorkerPool;	//进程表，从0开始
		wFile mPidFile;	//pid文件
		
		int mUseMutex;	//惊群锁标识
		int mMutexHeld;	//是否持有锁
		int mDelay;		//延时时间。默认500ms
		wShm *mShmAddr;	//共享内存
		wShmtx *mMutex;	//accept mutex
};

template <typename T>
wMaster<T>::wMaster()
{
	Initialize();
}

template <typename T>
wMaster<T>::~wMaster() 
{
	for (int i = 0; i < mWorkerNum; ++i)
	{
		SAFE_DELETE_VEC(mWorkerPool[i]);	//delete []mWorkerPool[i];
	}
	SAFE_DELETE_VEC(mWorkerPool);	//delete []mWorkerPool;

	SAFE_DELETE(mShmAddr);
	SAFE_DELETE(mMutex);
}

template <typename T>
wWorker* wMaster<T>::NewWorker(int iSlot) 
{
	return new wWorker(iSlot); 
}

template <typename T>
void wMaster<T>::Initialize()
{
	mSlot = 0;
	mWorkerPool = NULL;
	mShmAddr = NULL;
	mMutex = NULL;
	mUseMutex = 1;
	mMutexHeld = 0;
	mDelay = 500;
	mPid = getpid();
	mNcpu = sysconf(_SC_NPROCESSORS_ONLN);
	mProcess = PROCESS_SINGLE;
}

template <typename T>
void wMaster<T>::PrepareRun()
{
	//
}

template <typename T>
void wMaster<T>::Run()
{
	//
}

template <typename T>
void wMaster<T>::PrepareStart()
{
	mWorkerNum = mNcpu;	//默认 worker 数量等于cpu数量
	mPid = getpid();

	PrepareRun();	//初始化服务器

	InitSignals();	//初始化信号处理
	
	CreatePidFile();//创建pid文件
}

template <typename T>
void wMaster<T>::InitSignals()
{
	wSignal::signal_t *pSig;
	wSignal stSignal;
	for (pSig = g_signals; pSig->mSigno != 0; ++pSig)
	{
		if(stSignal.AddSig_t(pSig) != -1)
		{
			LOG_ERROR(ELOG_KEY, "[runtime] sigaction(%s) failed, ignored", pSig->mSigname);
		}
	}
}

template <typename T>
void wMaster<T>::SingleStart()
{
	mProcess = PROCESS_SINGLE;
	Run();
}

template <typename T>
void wMaster<T>::MasterStart()
{
	mProcess = PROCESS_MASTER;
	if (mWorkerNum > MAX_PROCESSES)
	{
		LOG_ERROR(ELOG_KEY, "[runtime] no more than %d processes can be spawned:", mWorkerNum);
		return;
	}

	//初始化workerpool内存空间
	mWorkerPool = new wWorker*[mWorkerNum];
	for(int i = 0; i < mWorkerNum; ++i)
	{
		mWorkerPool[i] = NewWorker(i);
	}
	
	//信号阻塞
	wSigSet stSigset;
	stSigset.AddSet(SIGCHLD);
	stSigset.AddSet(SIGALRM);
	stSigset.AddSet(SIGIO);
	stSigset.AddSet(SIGINT);
	stSigset.AddSet(SIGQUIT);
	stSigset.AddSet(SIGTERM);
	stSigset.AddSet(SIGHUP);	//RECONFIGURE
	stSigset.AddSet(SIGUSR1);	//

    if (stSigset.Procmask() == -1) 
    {
        LOG_ERROR(ELOG_KEY, "[runtime] sigprocmask() failed: %s", strerror(errno));
		return;
    }
    stSigset.EmptySet();
	
	//防敬群锁
	if(mUseMutex == 1)
	{
		mShmAddr = new wShm(WAIT_MUTEX, 'a', sizeof(wShmtx));
		mMutex = new wShmtx();
		mMutex->Create(mShmAddr);
	}
	
    //启动worker进程
    WorkerStart(mWorkerNum, PROCESS_RESPAWN);

	struct itimerval itv;
    int delay = 0;
    int live = 1;

	//master进程 信号处理
	while (true)
	{
		//
		stSigset.Suspend();

		HandleSignal();

		Run();
	}
}

template <typename T>
void wMaster<T>::HandleSignal()
{
		if(g_reap)
		{
			g_reap = 0;
			//
		}

		if(g_terminate)
		{
			//
		}

		if(g_quit)
		{
			//
		}

		if (g_reconfigure)
		{
			//
		}

		//收到SIGUSR1信号，重新打开log文件
		if (g_reopen)
		{
			//
		}
}

template <typename T>
void wMaster<T>::WorkerStart(int n, int type)
{
	const char *sProcessTitle = "worker process";
	pid_t pid;
	
	//同步channel fd消息结构
	struct ChannelReqOpen_t stCh;
	memset(&stCh, 0, sizeof(struct ChannelReqOpen_t));
	
	for (int i = 0; i < mWorkerNum; ++i)
	{
		//创建worker进程
		pid = SpawnWorker(i, sProcessTitle, type);
	
		stCh.mSlot = mSlot;
        stCh.mPid = mWorkerPool[mSlot]->mPid;
        stCh.mFD = mWorkerPool[mSlot]->mCh[0];
        PassOpenChannel(&stCh);
	}
}

template <typename T>
void wMaster<T>::PassOpenChannel(struct ChannelReqOpen_t* pCh)
{
	int size = sizeof(struct ChannelReqOpen_t);
	char *pBuf = new char[size + sizeof(int)];
	*(int *)pBuf = size;	//消息长度
	pBuf += sizeof(int);
    
	for (int i = 0; i < mWorkerNum; i++) 
    {
		//当前分配到worker进程表项索引（无需发送给自己）
        if (i == mSlot|| mWorkerPool[i]->mPid == -1|| mWorkerPool[i]->mCh[0] == FD_UNKNOWN)
        {
            continue;
        }

        LOG_DEBUG(ELOG_KEY, "[runtime] pass channel s:%d pid:%P fd:%d to s:%i pid:%P fd:%d", 
        	pCh->mSlot, pCh->mPid, pCh->mFD, i, mWorkerPool[i]->mPid, mWorkerPool[i]->mCh[0]);
        
        /* TODO: EAGAIN */
		memcpy(pBuf, (char *)pCh, size);
		mWorkerPool[i]->mCh.SendBytes(pBuf, size + sizeof(int));
    }

    SAFE_DELETE(pBuf);
}

template <typename T>
pid_t wMaster<T>::SpawnWorker(int i, const char *title, int type)
{
	int s;
	for (s = 0; s < mWorkerNum; ++s)
	{
		if(mWorkerPool[s]->mPid == -1)
		{
			break;
		}
	}
	mSlot = s;	//当前已分配索引

	wWorker *pWorker = mWorkerPool[mSlot];
	if(pWorker->InitChannel() < 0)
	{
		LOG_ERROR(ELOG_KEY, "[runtime] socketpair() failed while spawning: %s", strerror(errno));
		return -1;
	}

	//设置第一个描述符的异步IO通知机制（FIOASYNC现已被O_ASYNC标志位取代）
	u_long on = 1;
    if (ioctl(pWorker->mCh[0], FIOASYNC, &on) == -1) 
    {
        LOG_ERROR(ELOG_KEY, "[runtime] ioctl(FIOASYNC) failed while spawning \"%s\":", title, strerror(errno));
        pWorker->Close();
        return -1;
    }

    //设置将要在文件描述符channel[0]上接收SIGIO 或 SIGURG事件信号的进程标识
    if (fcntl(pWorker->mCh[0], F_SETOWN, mPid) == -1) 
    {
        LOG_ERROR(ELOG_KEY, "[runtime] fcntl(F_SETOWN) failed while spawning \"%s\":", title, strerror(errno));
        pWorker->Close();
        return -1;
    }

    pid_t pid = fork();
    switch (pid) 
    {
	    case -1:
	        LOG_ERROR(ELOG_KEY, "[runtime] fork() failed while spawning \"%s\":", title, strerror(errno));
	        pWorker->Close();
	        return -1;
			
	    case 0:
	    	//worker进程
	        pWorker->InitWorker(mWorkerNum, mWorkerPool, mUseMutex, mShmAddr, mMutex, mDelay);
	        pWorker->PrepareStart(type, (void *) &mSlot);
	        pWorker->Start();
	        _exit(0);	//TODO 进程退出

	        break;

	    default:
	        break;
    }

    LOG_DEBUG(ELOG_KEY, "[runtime] start %s %P", title, pid);
    
    //更新进程表
    pWorker->mSlot = mSlot;
    pWorker->mPid = pid;
    switch (type)
    {
    	case PROCESS_NORESPAWN:
    		pWorker->mRespawn = 1;
    		break;

    	case PROCESS_RESPAWN:
    		pWorker->mRespawn = 1;

    		break;
    	case PROCESS_DETACHED:
    		pWorker->mDetached = 1;
    }

    //
    return pid;
}

template <typename T>
int wMaster<T>::CreatePidFile()
{
	if (mPidFile.FileName().size() <= 0)
	{
		mPidFile.FileName() = "master.pid";
	}
    if (mPidFile.Open(O_RDWR| O_CREAT) <= 0) 
    {
    	LOG_ERROR(ELOG_KEY, "[runtime] create pid file failed");
    	return -1;
    }
	string sPid = Itos((int) mPid);
    if (mPidFile.Write(sPid.c_str(), sPid.size(), 0) == -1) 
    {
		LOG_ERROR(ELOG_KEY, "[runtime] write process pid to file failed");
        return -1;
    }
	
    mPidFile.Close();
    return 0;
}

template <typename T>
void wMaster<T>::DeletePidFile()
{
    if (mPidFile.Unlink() == -1) 
    {
    	LOG_ERROR(ELOG_KEY, "unlink \"%s\" failed", mPidFile.FileName().c_str());
		return;
    }
	return;
}

#endif