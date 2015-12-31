
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_MPROCESS_H_
#define _W_MPROCESS_H_

#include <unistd.h>
#include <map>
#include <vector>
#include <string>
#include <string.h>

#include "wType.h"
#include "wLog.h"
#include "wNoncopyable.h"
#include "wProcess.h"
#include "wShareMemory.h"
#include "wMsgQueue.h"

class wMProcess : private wNoncopyable
{
	public:
		wMProcess();
		virtual ~wMProcess();

		int PrepareStart();
		int Start(int iNum = 8);

		virtual wProcess* NewProcess(int iIdx, string sTitle);
		pid_t GenerateWorker(int iIdx, string sTitle);
	protected:
		map<int, wProcess*> mProcList;
		string mName;
		pid_t mPid;

		wShareMemory* mShm;
};

#endif