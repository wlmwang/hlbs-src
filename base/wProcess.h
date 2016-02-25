
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_PROCESS_H_
#define _W_PROCESS_H_

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

		virtual void PrepateRun();
		virtual void Run();

	protected:
		pid_t mPid;
		string mName;

		wShm* mShm;
};

wProcess* G_pProc;

#endif