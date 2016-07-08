
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _W_WORKER_H_
#define _W_WORKER_H_

#include <map>
#include <vector>

#include "wCore.h"
#include "wLog.h"
#include "wMisc.h"
#include "wSigSet.h"
#include "wSignal.h"
#include "wFile.h"
#include "wChannel.h"
#include "wShm.h"
#include "wShmtx.h"
#include "wNoncopyable.h"
#include "wChannelCmd.h"

class wWorker : public wNoncopyable
{
	public:
		wWorker(int iSlot = 0);
		virtual ~wWorker() {}

		/**
		 * 设置进程标题
		 */
		virtual void PrepareRun() {}
		virtual void Run() {}
		virtual void Close();

		void InitWorker(int iWorkerNum = 0, wWorker **pWorkerPool = NULL, int iUseMutex = 1, wShm *pShmAddr = NULL, wShmtx *pMutex = NULL, int iDelay = 500);
		void PrepareStart(int iSlot, int iType, const char *pTitle, void *pData);
		void Start(bool bDaemon = true);

		int InitChannel();
	
	public:
		int mErr;
		pid_t mPid {-1};
		uid_t mUid {0};
		gid_t mGid {0};
		int mPriority {0};			//进程优先级
		int mRlimitCore {1024};		//连接限制
		char mWorkingDir[255] {'\0'};	//工作目录
		
		char *mName {NULL};	//进程名
		void *mData {NULL};	//进程参数
		int mDetached {0};	//是否已分离
		int mExited {0};	//已退出 进程表mWorkerPool已回收
		int mExiting {0};	//正在退出
		int mStat {0};		//waitpid子进程退出状态
		int mRespawn {PROCESS_NORESPAWN};	//worker启动模式。退出是否重启
		int mJustSpawn {PROCESS_JUST_SPAWN};

		WORKER_STATUS mStatus {WORKER_INIT};

		//进程表相关，对应 wMaster 相关属性
		int mSlot {0};
		int mWorkerNum {0};
		wWorker **mWorkerPool {NULL};

		//惊群锁相关，对应 wMaster 相关属性
		int mUseMutex {0};
		wShm *mShmAddr {NULL};
		wShmtx *mMutex {NULL};
		int mDelay {0};
		int mMutexHeld {0};

		wChannel mCh;	//worker进程channel
};

#endif