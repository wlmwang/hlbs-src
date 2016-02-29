
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
		wShmtx() {}
		
		void Initialize()
		{
			mSem = NULL;
			mSpin = 0;
		}

		virtual ~wShmtx() {}

		/**
		 * 创建锁（在共享建立sem）
		 * @param  pShm [共享内存]
		 * @param  iSpin [自旋初始值]
		 * @return       [0]
		 */
		int Create(wShm *pShm, int iSpin = 2048)
		{
			char *pAddr = pShm->AllocShm(sizeof(wSem));
			if (pAddr == 0)
			{
				LOG_ERROR(ELOG_KEY, "shm alloc failed: %d", sizeof(wSem));
				exit(-1);
			}
			mSem = (wSem *) pAddr;
			mSpin = iSpin;
			return 0;
		}
		
		int Lock()
		{
			if (mSem == NULL)
			{
				return -1;
			}
			return mSem->Wait();
		}

		int TryLock()
		{
			if (mSem == NULL)
			{
				return -1;
			}
			return mSem->TryWait();
		}

		//自旋争抢锁
		void LockSpin()
		{
		    int	i, n;

		    int ncpu = sysconf(_SC_NPROCESSORS_ONLN);   //cpu个数

		    while (true) 
		    {
		        if (mSem->TryWait() == 0)
		        {
		        	return;
		        }

		        if (ncpu > 1) 
		        {
		            for (n = 1; n < mSpin; n <<= 1) 
		            {
		                for (i = 0; i < n; i++) 
		                {
		                    pause();	//暂停
		                }

				        if (mSem->TryWait() == 0)
				        {
				        	return;
				        }
		            }
		        }
		        sched_yield();	//usleep(1);
		    }
		}

	private:
		wSem *mSem;
		int  mSpin;	
};

#endif