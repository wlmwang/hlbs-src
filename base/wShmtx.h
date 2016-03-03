
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_SHMTX_H_
#define _W_SHMTX_H_

#include "wType.h"
#include "wLog.h"
#include "wNoncopyable.h"
#include "wShm.h"
#include "wSem.h"

/**
 * 多进程互斥量（主要用于父子进程）
 * 基于信号量实现
 */
class wShmtx : private wNoncopyable
{
	public:
		wShmtx();
		virtual ~wShmtx();
		void Initialize();

		/**
		 * 创建锁（在共享建立sem）
		 * @param  pShm [共享内存]
		 * @param  iSpin [自旋初始值]
		 * @return       [0]
		 */
		int Create(wShm *pShm, int iSpin = 2048);
		
		int Lock();

		int TryLock();

		//自旋争抢锁
		void LockSpin();

	private:
		wSem *mSem;
		int  mSpin;	
};

#endif