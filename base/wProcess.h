
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_PROCESS_H_
#define _W_PROCESS_H_

#define PROCESS_SINGLE     0 	//单独进程
#define PROCESS_MASTER     1 	//主进程
#define PROCESS_SIGNALLER  2 	//信号进程
#define PROCESS_WORKER     3 	//工作进程
//#define PROCESS_HELPER     4 	//辅助进程


#define PROCESS_NORESPAWN     -1	//子进程退出时，父进程不再创建
#define PROCESS_JUST_SPAWN    -2
#define PROCESS_RESPAWN       -3	//子进程异常退出时，master会重新创建它。如当worker或cache manager异常退出时，父进程会重新创建它

#include "wType.h"
#include "wLog.h"
#include "wNoncopyable.h"
#include "wShm.h"
#include "wMsgQueue.h"

class wMProcess;
class wProcess : private wNoncopyable
{
	friend class wMProcess;
	public:
		wProcess(string sName);
		virtual ~wProcess();

		virtual void PrepareRun();
		virtual void Run();

	protected:
		pid_t mPid;
		string mName;

		wShm* mShm;
};

wProcess* G_pProc;

#endif