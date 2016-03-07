
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_WORKER_H_
#define _W_WORKER_H_

#include <map>
#include <vector>

#include "wType.h"
#include "wLog.h"
#include "wMisc.h"
#include "wSigSet.h"
#include "wSignal.h"
#include "wFile.h"
#include "wChannel.h"
#include "wShm.h"
#include "wShmtx.h"
#include "wNoncopyable.h"
#include "wBaseCmd.h"

class wWorker : public wNoncopyable
{
	public:
		wWorker(int iSlot = 0);
		void Initialize();
		virtual ~wWorker();

		/**
		 * 设置进程标题
		 */
		virtual void PrepareRun();
		virtual void Run();
		virtual void Close();

		void InitWorker(int iWorkerNum = 0, wWorker **pWorkerPool = NULL, int iUseMutex = 1, wShm *pShmAddr = NULL, wShmtx *pMutex = NULL, int iDelay = 500);
		void PrepareStart(int iSlot, int iType, void *pData);
		void Start(bool bDaemon = true);

		int InitChannel();
	
	public:
		int mProcess;
		pid_t mPid;
		uid_t mUid;
		gid_t mGid;
		int mPriority;			//进程优先级
		int mRlimitCore;		//连接限制
		char mWorkingDir[255];	//工作目录
		
		void* mData;	//进程参数
		int mDetached;	//是否已分离
		int mExited;	//已退出
		int mExiting;	//正在退出
		int mRespawn;	//worker启动模式。退出是否重启

		WORKER_STATUS mStatus;

		//进程表相关，对应 wMaster 相关属性
		int mSlot;
		int mWorkerNum;
		wWorker **mWorkerPool;

		//惊群锁相关，对应 wMaster 相关属性
		int mUseMutex;
		wShm *mShmAddr;
		wShmtx *mMutex;
		int mDelay;
		int mMutexHeld;

		wChannel mCh;	//worker进程channel
};

#endif