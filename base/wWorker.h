
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
#include "wSigSet.h"
#include "wChannel.h"
#include "wNoncopyable.h"
#include "wMaster.h"

class wWorker : public wNoncopyable
{
	public:
		wWorker();
		void Initialize(int iWorkerNum = 0, wWorker **pWorkerPool = NULL, int iUseMutex = 1, wShm *pShmAddr = NULL, wShmtx *pMutex = NULL);
		virtual ~wWorker();

		virtual void PrepareRun();
		virtual void Run();
		virtual void Close();

		void PrepareStart(int type, void *data);
		void Start();

		int InitChannel();
	public:
		wChannel mCh;	//worker进程channel
		
		pid_t mPid;
		int mSlot;
		int mWorkerNum;
		wWorker **mWorkerPool;	//进程表，从0开始
		int mUseMutex;
		wShm *mShmAddr;
		wShmtx mMutex;	//accept mutex

		int mExited;
		int mRespawn;	//退出是否重启
};

#endif