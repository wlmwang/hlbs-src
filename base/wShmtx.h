
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_SHMTX_H_
#define _W_SHMTX_H_

#include <sys/ipc.h>
#include <sys/shm.h>

#include "wType.h"
#include "wLog.h"
#include "wNoncopyable.h"
#include "wShm.h"

/**
 * 进程间锁
 * 基于原子操作实现
 */
class wShmtx : private wNoncopyable
{
	public:
		wShmtx() {}

		virtual ~wShmtx() {}

		/**
		 * 创建锁（将贡献内存地址赋值给mLock地址）
		 * @param  pShm [共享内存]
		 * @param  iSpin [自旋初始值]
		 * @return       [0]
		 */
		int Create(wShm *pShm, int iSpin = 2048)
		{
			mLock = (int *) pShm->mAddr;
			mSpin = iSpin;
			return 0;
		}

		int TryLock()
		{
			return (*mLock == 0 && __sync_bool_compare_and_swap(mLock, 0, getpid()));
		}

		//阻塞方式获取锁
		void Lock()
		{
		    int	i, n;

		    for ( ;; ) 
		    {

		        if (*mLock == 0 && __sync_bool_compare_and_swap(mLock, 0, getpid())) 
		        {
		            return;
		        }

		        if (ngx_ncpu > 1) 
		        {

		            for (n = 1; n < mSpin; n <<= 1) 
		            {

		                for (i = 0; i < n; i++) 
		                {
		                    __asm__ ("pause");
		                }

		                if (*mLock == 0 && __sync_bool_compare_and_swap(mLock, 0, getpid()))
		                {
		                    return;
		                }
		            }
		        }
		        sched_yield();	//usleep(1);
		    }
		}

	private:
		int  *mLock;	//原子操作变量
		int  mSpin;	//自旋锁标识
}