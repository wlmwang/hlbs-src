
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
		wWorker() 
		{
			Initialize();
		}
		
		virtual void PrepareRun() {}
		virtual void Run() {}

		virtual ~wWorker() {}
		void Initialize();

		int InitChannel()
		{
			return mCh.Open();
		}
		
		virtual int InitWorker(int type, void *data);

		int PrepareStart() 
		{
			PrepareRun();
		}

		int Start() 
		{
			Run();
		}

	//private:
	public:
		wChannel mCh;	//worker进程channel
		int mSlot;
		pid_t mPid;
		int mExited;
		void *mData;
		int mRespawn;	//退出是否重启
};

#endif