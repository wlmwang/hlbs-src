
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */
 
#include "wShareMemory.h"

wShareMemory::wShareMemory(const char *filename, int pipeid = 'i', size_t size)
{
	mFilename = filename;
	mPipeId = pipeid;
	mSize = size;
}

wShareMemory::wShareMemory()
{
	RemoveShareMemory();
}

char *wShareMemory::CreateShareMemory()
{
	LOG_DEBUG("default", "try to alloc %lld bytes of share memory", mSize);
	//open(mFilename,);
	
	mKey = ftok(mFilename, mPipeId);
	if (mKey < 0) 
	{
		LOG_ERROR("default", "create memory (ftok) failed: %s", strerror(errno));
		return 0;
	}

	//申请共享内存
	mShmId = shmget(mKey, mSize, IPC_CREAT|IPC_EXCL|0666);
	
	//如果申请内存失败
	if (mShmId < 0) 
	{
		if (errno != EEXIST) 
		{
			LOG_ERROR("default", "Alloc share memory failed: %s", strerror(errno));
			return 0;
		}

		LOG_DEBUG("default", "share memory is exist now, try to attach it");

		//如果该内存已经被申请，则申请访问控制它
		mShmId = shmget(mKey, mSize, 0666);

		//如果失败
		if (mShmId < 0) 
		{
			LOG_DEBUG("default", "attach to share memory failed: %s, try to touch it", strerror(errno));
			
			//猜测是否是该内存大小太小，先获取内存ID
			mShmId = shmget(mKey, 0, 0666);
			
			//如果失败，则无法操作该内存，只能退出
			if (mShmId < 0) 
			{
				LOG_ERROR("default", "touch to share memory failed: %s", strerror(errno));
				return 0;
			}
			else 
			{
				LOG_DEBUG("default", "remove the exist share memory %d", mShmId);

				//如果成功，则先删除原内存
				if (shmctl(mShmId, IPC_RMID, NULL) < 0) 
				{
					LOG_ERROR("default", "remove share memory failed: %s", strerror(errno));
					return 0;
				}

				//再次申请该ID的内存
				mShmId = shmget(mKey, mSize, IPC_CREAT|IPC_EXCL|0666);
				if (mShmId < 0) 
				{
					LOG_ERROR("default", "alloc share memory failed again: %s", strerror(errno));
					return 0;
				}
			}
		}
		else
		{
			LOG_DEBUG("default", "attach to share memory succeed");
		}
	}

	LOG_INFO("default", "alloc %lld bytes of share memory succeed", mSize);
	
	mAddr = (char *)shmat(mShmId, NULL, 0);
    if (mAddr == (void *) -1) 
	{
		LOG_ERROR("default", "shmat() failed: %s", strerror(errno));
		return 0;
    }
	
	/*
    if (shmctl(mShmId, IPC_RMID, NULL) == -1) 
	{
		LOG_ERROR("default", "remove share memory failed: %s", strerror(errno));
		return 0;
    }
	*/
	return mAddr;
}

char *wShareMemory::AttachShareMemory()
{
	//把需要申请共享内存的key值申请出来
	mKey = ftok(mFilename, mPipeId);
	if (mKey < 0) 
	{
		LOG_ERROR("default", "create memory (ftok) failed: %s", strerror(errno));
		return 0;
	}

	// 尝试获取
	int mShmId = shmget(mKey, size, 0666);
	if( mShmId < 0 ) 
	{
		LOG_ERROR("default", "attach to share memory failed: %s", strerror(errno));
		return 0;
	}
	
	mAddr = (char *)shmat(mShmId, NULL, 0);
    if (mAddr == (void *) -1) 
	{
		LOG_ERROR("default", "shmat() failed: %s", strerror(errno));
		return 0;
    }
	
	/*
    if (shmctl(mShmId, IPC_RMID, NULL) == -1) 
	{
		LOG_ERROR("default", "remove share memory failed: %s", strerror(errno));
		return 0;
    }
	*/
	return mAddr;
}

void wShareMemory::RemoveShareMemory()
{
	if(mAddr ==0)
	{
		return;
	}
    if (shmdt(mAddr) == -1) 
	{
		LOG_ERROR("default", "shmdt(%d) failed", mAddr);
    }
	/*
    if (shmctl(mShmId, IPC_RMID, NULL) == -1) 
	{
		LOG_ERROR("default", "remove share memory failed: %s", strerror(errno));
    }
	*/
}
