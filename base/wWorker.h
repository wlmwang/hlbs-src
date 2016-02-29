
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
#include "wNoncopyable.h"
#include "wMaster.h"

class wWorker : public wNoncopyable
{
	friend class wMaster;
	public:
		wWorker() 
		{
			Initialize();
		}

		~wWorker() {}
		void Initialize() 
		{
			mPid = -1;
			mSlot = -1;
			mExited = -1;
			mData = NULL;
			mRespawn = 0;
		}

		int InitWorker()
		{
			//
		}

		int InitChannel()
		{
			mCh.Open();
		}

		int PrepareStart() {}

		int Start() 
		{
			//
		}

	private:
		wChannel mCh;	//worker进程channel
		int mSlot;
		pid_t mPid;
		int mExited;
		void *mData;
		int mRespawn;	//退出是否重启
};

#endif