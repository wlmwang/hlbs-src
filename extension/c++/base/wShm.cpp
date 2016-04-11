
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */
 
#include "wShm.h"

wShm::wShm(const char *filename, int pipeid, size_t size)
{
	Initialize();

	mPipeId = pipeid;
	mSize = size + sizeof(struct shmhead_t);
	if(mPagesize > 0)
	{
		mSize = ALIGN(mSize, mPagesize);
	}
	
	memcpy(mFilename, filename, strlen(filename) +1);
}

void wShm::Initialize()
{
	mPagesize = getpagesize();
	memset(mFilename, 0, sizeof(mFilename));
	mPipeId = 0;
	mSize = 0;
	mShmId = 0;
	mKey = 0;
	mShmhead = NULL;
}

wShm::~wShm()
{
	FreeShm();
}

char *wShm::CreateShm()
{	
	int iFD = open(mFilename, O_CREAT);
	if (iFD < 0)
	{
		return NULL;
	}
	close(iFD);

	mKey = ftok(mFilename, mPipeId);
	if (mKey < 0) 
	{
		return NULL;
	}

	//申请共享内存
	mShmId = shmget(mKey, mSize, IPC_CREAT| IPC_EXCL| 0666);
	
	//如果申请内存失败
	if (mShmId < 0) 
	{
		if (errno != EEXIST) 
		{
			return 0;
		}

		//如果该内存已经被申请，则申请访问控制它
		mShmId = shmget(mKey, mSize, 0666);

		//如果失败
		if (mShmId < 0) 
		{			
			//猜测是否是该内存大小太小，先获取内存ID
			mShmId = shmget(mKey, 0, 0666);
			
			//如果失败，则无法操作该内存，只能退出
			if (mShmId < 0) 
			{
				return 0;
			}
			else 
			{
				//如果成功，则先删除原内存
				if (shmctl(mShmId, IPC_RMID, NULL) < 0) 
				{
					return 0;
				}

				//再次申请该ID的内存
				mShmId = shmget(mKey, mSize, IPC_CREAT|IPC_EXCL|0666);
				if (mShmId < 0) 
				{
					return 0;
				}
			}
		}
	}
	
	char *pAddr = (char *)shmat(mShmId, NULL, 0);
    if (pAddr == (char *)-1) 
	{
		return 0;
    }

    //shm头
	mShmhead = (struct shmhead_t*) pAddr;
	mShmhead->mStart = pAddr;
	mShmhead->mEnd = pAddr + mSize;
	mShmhead->mUsedOff = pAddr + sizeof(struct shmhead_t);
	return mShmhead->mUsedOff;
}

char *wShm::AttachShm()
{	
	//把需要申请共享内存的key值申请出来
	mKey = ftok(mFilename, mPipeId);
	if (mKey < 0) 
	{
		return 0;
	}

	// 尝试获取
	int mShmId = shmget(mKey, mSize, 0666);
	if(mShmId < 0) 
	{
		return 0;
	}
	
	char *pAddr = (char *)shmat(mShmId, NULL, 0);
    if (pAddr == (char *) -1) 
	{
		return 0;
    }
	
    //shm头
	mShmhead = (struct shmhead_t*) pAddr;
	return mShmhead->mUsedOff;
}

char *wShm::AllocShm(size_t size)
{
	if (mShmhead == NULL)
	{
		return NULL;
	}

	if(mShmhead != NULL && mShmhead->mUsedOff + size < mShmhead->mEnd)
	{
		char *pAddr = mShmhead->mUsedOff;
		mShmhead->mUsedOff += size;
		memset(pAddr, 0, size);
		return pAddr;
	}
	return NULL;
}

void wShm::FreeShm()
{	
	if(mShmhead == NULL || mShmhead->mStart == NULL)
	{
		return;
	}

	//对共享操作结束，分离该shmid_ds与该进程关联计数器
    if (shmdt(mShmhead->mStart) == -1)
	{
    }
	
	//删除该shmid_ds共享存储段（全部进程结束才会真正删除）
    if (shmctl(mShmId, IPC_RMID, NULL) == -1)
	{
    }
	//unlink(mFilename);
}
