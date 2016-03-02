
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
#include "wWorker.h"
#include "wChannel.h"
#include "wSigSet.h"
#include "wSignal.h"

#define PROCESS_SINGLE     0 	//单独进程
#define PROCESS_MASTER     1 	//主进程
#define PROCESS_SIGNALLER  2 	//信号进程
#define PROCESS_WORKER     3 	//工作进程
//#define PROCESS_HELPER     4 	//辅助进程


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
		void MasterStart();		//master-worker启动
		void SingleStart();		//单进程启动
		
		void WorkerStart(int n, int type = PROCESS_RESPAWN);

		virtual void PrepareRun();
		virtual void Run();

		pid_t SpawnWorker(int i, const char *title, int type = PROCESS_RESPAWN);

		virtual wWorker* NewWorker(int iSlot = 0);

		int CreatePidFile();

		void DeletePidFile();

	protected:
		wFile mPidFile;
		
		int mNcpu;
		pid_t mPid;		//master进程id
		int mWorkerNum;	//worker总数量
		int mSlot;		//进程表分配到数量
		wWorker **mWorkerPool;	//进程表，从0开始
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
}

template <typename T>
void wMaster<T>::Initialize()
{
	mSlot = 0;
	mWorkerPool = NULL;
	mPid = getpid();
	mNcpu = sysconf(_SC_NPROCESSORS_ONLN);
}

template <typename T>
wWorker* wMaster<T>::NewWorker(int iSlot) 
{
	return new wWorker(); 
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
	
	wSigSet stSigset;
	stSigset.AddSet(SIGCHLD);
	stSigset.AddSet(SIGALRM);
	stSigset.AddSet(SIGIO);
	stSigset.AddSet(SIGINT);
	stSigset.AddSet(SIGQUIT);
	stSigset.AddSet(SIGTERM);
	stSigset.AddSet(SIGHUP);	//RECONFIGURE
	stSigset.AddSet(SIGUSR2);	//CHANGEBIN

    if (stSigset.Procmask() == -1) 
    {
        LOG_ERROR(ELOG_KEY, "[runtime] sigprocmask() failed: %s", strerror(errno));
    }

	//pid文件
	CreatePidFile();

	PrepareRun();
}

template <typename T>
void wMaster<T>::MasterStart()
{
	struct itimerval   itv;

	wSigSet stSigset;

    //初始化workerpool
	if (mWorkerNum > MAX_PROCESSES)
	{
		LOG_ERROR(ELOG_KEY, "[runtime] no more than %d processes can be spawned:", mWorkerNum);
		return;
	}

	mWorkerPool = new wWorker*[mWorkerNum];
	for(int i = 0; i < mWorkerNum; ++i)
	{
		mWorkerPool[i] = NewWorker(i);
	}

    //启动worker进程
    WorkerStart(mWorkerNum, PROCESS_RESPAWN);

	//信号处理
	while (true)
	{
		Run();

		//退出处理（严格的延时退出）
		
		//阻塞方式等待信号量。会被上面设置的定时器打断
        stSigset.Suspend();

        //SIGCHLD
        //异常重启
	}
}

template <typename T>
void wMaster<T>::WorkerStart(int n, int type)
{
	pid_t pid;

	wChannel ch;
	memset(&ch, 0, sizeof(wChannel));

	//ch.mCommand = CMD_OPEN_CHANNEL;	//打开channel，新的channel

	for (int i = 0; i < mWorkerNum; ++i)
	{
		pid = SpawnWorker(i, "worker process", type);
	
        //ch.pid = mWorkerPool[mSlot].mPid;
        //ch.slot = mSlot;
        //ch.fd = mWorkerPool[mSlot].mCh[0];
        //pass_open_channel(cycle, &ch);	//发送此ch[0]到所有一创建的worker进程。
	}
}

template <typename T>
pid_t wMaster<T>::SpawnWorker(int i, const char *title, int type)
{
	int s = 0;
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

    //设置将要在文件描述符channel[0]上接收SIGIO 或 SIGURG事件信号的进程或进程组的标识
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
    pWorker->mExited = 0;
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
	/*
    mPidFile.Open(PID_PATH, O_RDWR| O_CREAT);
    if (mPidFile.FD() == -1) 
    {
    	LOG_DEBUG(ELOG_KEY, "[runtime] create pid file failed");
    	return -1;
    }

	string sPid = Itos((int) mPid);
    if (mPidFile.Write(sPid.c_str(), s.size(), 0) == -1) 
    {
        return -1;
    }
    mPidFile.Close();
	*/
    return 0;

}

template <typename T>
void wMaster<T>::DeletePidFile()
{
    if (mPidFile.Unlink() == -1) 
    {
    	LOG_ERROR(ELOG_KEY, "unlink \"%s\" failed", mPidFile.FileName().c_str());
    }
}

#endif