
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
#include "wConfig.h"
#include "wWorker.h"
#include "wChannel.h"

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
		wMaster()
		{
			Initialize();
		}

		void Initialize();

		virtual wWorker* NewWorker() { return 0; }
		
		virtual ~wMaster() {}

		virtual void PrepareRun() {}
		virtual void Run() {}

		virtual int InitMaster();
		
		int PrepareStart() {}

		//master-worker启动
		void MasterStart();

		pid_t SpawnWorker(int i, const char *title, int type);

		//单进程启动
		void SingleStart()
		{
			//
		}

		int CreatePidFile();

		void DeletePidFile();

	private:
		wFile mPidFile;
		
		int mNcpu;
		pid_t mPid;	//master进程id
		int mWorkerNum;	//worker总数量
		int mSlot;	//进程表分配到数量
		wWorker **mWorkerPool;	//进程表
};

template <typename T>
void wMaster<T>::Initialize()
{
	mNcpu = sysconf(_SC_NPROCESSORS_ONLN);
	mPid = getpid();
	mWorkerNum = mNcpu;
	mSlot = 0;
}

template <typename T>
int wMaster<T>::InitMaster()
{
	PrepareRun();

	if (mWorkerNum > MAX_PROCESSES)
	{
		LOG_ERROR(ELOG_KEY, "[runtime] no more than %d processes can be spawned:", mWorkerNum);
		return -1;
	}
	//初始化workerpool
	for(int i = 0; i < mWorkerNum; ++i)
	{
		mWorkerPool[i] = NewWorker();
	}

	return 0;
}

template <typename T>
void wMaster<T>::MasterStart()
{
	pid_t pid;

	wChannel ch;
	memset(&ch, 0, sizeof(wChannel));

	//ch.mCommand = CMD_OPEN_CHANNEL;	//打开channel，新的channel

	for (int i = 0; i < mWorkerNum; ++i)
	{
		pid = SpawnWorker(i, "worker process", PROCESS_RESPAWN);
	
        //ch.pid = mWorkerPool[mSlot].mPid;
        //ch.slot = mSlot;
        //ch.fd = mWorkerPool[mSlot].mCh[0];
        //pass_open_channel(cycle, &ch);	//发送此ch[0]到所有一创建的worker进程。
	}

	Run();
	//信号量
	
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
	mSlot = s;	//当前第几个

	wWorker *pWorker = mWorkerPool[mSlot];	//取出进程表

	pWorker->InitChannel();

	//设置第一个描述符的异步IO通知机制（FIOASYNC现已被O_ASYNC标志位取代）
	u_long on = 1;
    if (ioctl(pWorker->mCh[0], FIOASYNC, &on) == -1) 
    {
        LOG_ERROR(ELOG_KEY, "[runtime] ioctl(FIOASYNC) failed while spawning \"%s\":", title, strerror(errno));
        pWorker->mCh.Close();
        return -1;
    }

    //设置将要在文件描述符channel[0]上接收SIGIO 或 SIGURG事件信号的进程或进程组的标识
    if (fcntl(pWorker->mCh[0], F_SETOWN, mPid) == -1) 
    {
        LOG_ERROR(ELOG_KEY, "[runtime] fcntl(F_SETOWN) failed while spawning \"%s\":", title, strerror(errno));
        pWorker->mCh.Close();
        return -1;
    }
    
    pid_t pid = fork();

    switch (pid) 
    {
    case -1:
        LOG_ERROR(ELOG_KEY, "[runtime] fork() failed while spawning \"%s\":", title, strerror(errno));
        pWorker->mCh.Close();
        return -1;

    case 0:
    	pWorker->InitWorker(type, (void *) &mSlot);
        pWorker->PrepareStart();
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