
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

#define PROCESS_SINGLE     0 	//单独进程
#define PROCESS_MASTER     1 	//主进程
#define PROCESS_SIGNALLER  2 	//信号进程
#define PROCESS_WORKER     3 	//工作进程


#define PROCESS_NORESPAWN     -1	//子进程退出时，父进程不再创建
#define PROCESS_JUST_SPAWN    -2
#define PROCESS_RESPAWN       -3	//子进程异常退出时，master会重新创建它。如当worker或cache manager异常退出时，父进程会重新创建它

#define MAX_PROCESSES         1024

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
		void PassOpenChannel(wChannel::channel_t *ch);
		virtual wWorker* NewWorker(int iSlot = 0);
		
		virtual void PrepareRun();
		virtual void Run();
		
		/**
		 *  注册信号回调
		 *  可重写全局变量g_signals，实现自定义信号处理
		 */
		void InitSignals();
		
		int CreatePidFile();
		void DeletePidFile();

	protected:
	
		int mNcpu;		//cpu个数
		pid_t mPid;		//master进程id
		int mSlot;		//进程表分配到数量
		int mWorkerNum;	//worker总数量
		wWorker **mWorkerPool;	//进程表，从0开始
		wFile mPidFile;	//pid文件
		
		int mUseMutex;
		wShm *mShmAddr;
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
	mPid = getpid();
	mNcpu = sysconf(_SC_NPROCESSORS_ONLN);
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
	mWorkerNum = mNcpu;
	mPid = getpid();

	PrepareRun();

	InitSignals();
	
	CreatePidFile();
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
	Run();
}

template <typename T>
void wMaster<T>::MasterStart()
{
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
		Run();
	}
}

template <typename T>
void wMaster<T>::WorkerStart(int n, int type)
{
	const char *sProcessTitle = "worker process";
	pid_t pid;
	
	struct ChannelReqOpen_t stCh;
	memset(&stCh, 0, sizeof(struct ChannelReqOpen_t));
	
	for (int i = 0; i < mWorkerNum; ++i)
	{
		//创建worker进程
		pid = SpawnWorker(i, sProcessTitle, type);
	
		stCh.mSlot = mSlot;
        stCh.mPid = mWorkerPool[mSlot]->mPid;
        stCh.mFD = mWorkerPool[mSlot]->mCh[0];

        PassOpenChannel(&stCh);	//发送此ch[0]给所有已创建的worker进程
	}
}

template <typename T>
void wMaster<T>::PassOpenChannel(struct ChannelReqOpen_t* pCh)
{
	int size = sizeof(struct ChannelReqOpen_t);
	char *pBuf = new char[size + sizeof(int)];
	*(int *)pBuf = size;
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
		memcpy(pBuf, pCh, size);
		mWorkerPool[i]->mCh.SendBytes(pBuf, size + sizeof(int));
    }
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
	mSlot = s;	//当前进程索引

	wWorker *pWorker = mWorkerPool[mSlot];	//取出进程表

	if(pWorker->InitChannel() == -1)
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
	    	//初始化
	    	pWorker->Initialize(mWorkerNum, mWorkerPool, mUseMutex, mShmAddr, mMutex);
	        pWorker->PrepareStart(type, (void *) &mSlot);
	        pWorker->Start();
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
    	case PROCESS_RESPAWN:
    		pWorker->mRespawn = 1;
    }
    return pid;
}

template <typename T>
int wMaster<T>::CreatePidFile()
{
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