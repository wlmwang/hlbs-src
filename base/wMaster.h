
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_MASTER_H_
#define _W_MASTER_H_

#include <map>
#include <vector>

#include "wCore.h"
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
#include "wChannelCmd.h"

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
		void MasterExit();
		
		void WorkerStart(int n, int type = PROCESS_RESPAWN);
		pid_t SpawnWorker(void* pData, const char *title, int type = PROCESS_RESPAWN);
		void PassOpenChannel(struct ChannelReqOpen_t *pCh);
		void PassCloseChannel(struct ChannelReqClose_t *pCh);
		virtual wWorker* NewWorker(int iSlot = 0);
		virtual void HandleSignal();
		/**
		 *  如果有worker异常退出，则重启
		 *  如果所有的worker都退出了，则返回0
		 */
		int ReapChildren();
		/**
		 *  给所有worker进程发送信号
		 */
		void SignalWorker(int iSigno);

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
		/**
		 *  回收退出进程状态（waitpid以防僵尸进程）
		 *  更新进程表
		 */
		void ProcessGetStatus();
		
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

		int mErr;
};

#include "wMaster.inl"

#endif