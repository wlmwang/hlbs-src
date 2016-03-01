
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include "wShmtx.h"

void wShmtx::Initialize()
{
	mSem = NULL;
	mSpin = 0;
}

int wShmtx::Create(wShm *pShm, int iSpin)
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

int wShmtx::Lock()
{
	if (mSem == NULL)
	{
		return -1;
	}
	return mSem->Wait();
}

int wShmtx::TryLock()
{
	if (mSem == NULL)
	{
		return -1;
	}
	return mSem->TryWait();
}

void wShmtx::LockSpin()
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
