
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _W_THREAD_POOL_H_
#define _W_THREAD_POOL_H_

#include "wCore.h"
#include "wMutex.h"
#include "wNoncopyable.h"
#include "wThread.h"

class wThreadPool : private wNoncopyable
{
	public:
		wThreadPool();
		virtual ~wThreadPool();
		void Initialize();
		void CleanPool();
		
		void PrepareStart();
		void Start(bool bDaemon = true);
		
		virtual int AddPool(wThread* pThread);
		virtual int DelPool(wThread* pThread);
		
		virtual void PrepareRun();
		virtual void Run();
		
	protected:
		vector<wThread *> mThreadPool;
		wMutex *mMutex;
		wCond *mCond;
};

#endif
