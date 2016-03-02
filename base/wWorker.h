
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
#include "wChannel.h"
#include "wNoncopyable.h"

class wWorker : public wNoncopyable
{
	//friend class wMaster;
	public:
		wWorker();
		void Initialize();
		virtual ~wWorker();

		virtual void PrepareRun();
		virtual void Run();
		virtual void Close();

		void PrepareStart(int type, void *data);
		void Start();

		int InitChannel();

	//protected:
	public:
		wChannel mCh;	//worker进程channel
		
		int mSlot;
		pid_t mPid;
		int mExited;
		int mRespawn;	//退出是否重启
};

#endif