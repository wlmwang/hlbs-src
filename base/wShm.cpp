
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */
 
#include "wShm.h"

wShm::wShm(const char *filename, int pipeid, size_t size)
{
	Initialize();
	mPipeId = pipeid;
	int pagesize = getpagesize();
	if(pagesize > 0)
	{
		mSize = ALIGN(size, pagesize);
	}
	else
	{
		mSize = size;
	}
	memcpy(mFilename, filename, strlen(filename) +1);
}

void wShm::Initialize()
{
	memset(mFilename, 0, sizeof(mFilename));
	mPipeId = 0;
	mSize = 0;
	mStart = mEnd = mPos = 0;
}

wShm::~wShm()
{
	RemoveShm();
}

char *wShm::CreateShm()
{
	LOG_DEBUG(ELOG_KEY, "[runtime] try to alloc %lld bytes of share memory", mSize);
	
	int fd = open(mFilename, O_CREAT);
	if (fd < 0)
	{
		LOG_ERROR(ELOG_KEY, "[runtime] open file(%s) failed: %s", mFilename, strerror(errno));
		return NULL;
	}
	close(fd);

	mKey = ftok(mFilename, mPipeId);
	if (mKey < 0) 
	{
		LOG_ERROR(ELOG_KEY, "[runtime] create memory (ftok) failed: %s", strerror(errno));
		return NULL;
	}

	//申请共享内存
	mShmId = shmget(mKey, mSize, IPC_CREAT| IPC_EXCL| 0666);
	
	//如果申请内存失败
	if (mShmId < 0) 
	{
		if (errno != EEXIST) 
		{
			LOG_ERROR(ELOG_KEY, "[runtime] alloc share memory failed: %s", strerror(errno));
			return 0;
		}

		LOG_DEBUG(ELOG_KEY, "[runtime] share memory is exist now, try to attach it");

		//如果该内存已经被申请，则申请访问控制它
		mShmId = shmget(mKey, mSize, 0666);

		//如果失败
		if (mShmId < 0) 
		{
			LOG_DEBUG(ELOG_KEY, "[runtime] attach to share memory failed: %s, try to touch it", strerror(errno));
			
			//猜测是否是该内存大小太小，先获取内存ID
			mShmId = shmget(mKey, 0, 0666);
			
			//如果失败，则无法操作该内存，只能退出
			if (mShmId < 0) 
			{
				LOG_ERROR(ELOG_KEY, "[runtime] touch to share memory failed: %s", strerror(errno));
				return 0;
			}
			else 
			{
				LOG_DEBUG(ELOG_KEY, "[runtime] remove the exist share memory %d", mShmId);

				//如果成功，则先删除原内存
				if (shmctl(mShmId, IPC_RMID, NULL) < 0) 
				{
					LOG_ERROR(ELOG_KEY, "[runtime] remove share memory failed: %s", strerror(errno));
					return 0;
				}

				//再次申请该ID的内存
				mShmId = shmget(mKey, mSize, IPC_CREAT|IPC_EXCL|0666);
				if (mShmId < 0) 
				{
					LOG_ERROR(ELOG_KEY, "[runtime] alloc share memory failed again: %s", strerror(errno));
					return 0;
				}
			}
		}
		else
		{
			LOG_DEBUG(ELOG_KEY, "[runtime] attach to share memory succeed");
		}
	}

	LOG_DEBUG(ELOG_KEY, "[runtime] alloc %lld bytes of share memory succeed", mSize);
	
	mStart = mPos = (char *)shmat(mShmId, NULL, 0);
    if (mStart == (void *) -1) 
	{
		LOG_ERROR(ELOG_KEY, "[runtime] shmat failed: %s", strerror(errno));
		return 0;
    }
	
	mEnd = mStart + mSize;
	return mStart;
}

char *wShm::AttachShm()
{
	LOG_DEBUG(ELOG_KEY, "[runtime] try to attach %lld bytes of share memory", mSize);
	
	//把需要申请共享内存的key值申请出来
	mKey = ftok(mFilename, mPipeId);
	if (mKey < 0) 
	{
		LOG_ERROR(ELOG_KEY, "create memory (ftok) failed: %s", strerror(errno));
		return 0;
	}

	// 尝试获取
	int mShmId = shmget(mKey, mSize, 0666);
	if( mShmId < 0 ) 
	{
		LOG_ERROR(ELOG_KEY, "attach to share memory failed: %s", strerror(errno));
		return 0;
	}
	
	mStart = mPos = (char *)shmat(mShmId, NULL, 0);
    if (mStart == (void *) -1) 
	{
		LOG_ERROR(ELOG_KEY, "shmat() failed: %s", strerror(errno));
		return 0;
    }
	
	mEnd = mStart + mSize;
	return mStart;
}

char *Alloc(size_t len)
{
	if(mPos + len < mEnd)
	{
		char *pAddr = mPos;
		mPos += len;
		return pAddr;
	}
	LOG_ERROR(ELOG_KEY, "shmat() failed: %s", strerror(errno));
	return 0;
}

void wShm::RemoveShm()
{
	if(mStart == 0)
	{
		return;
	}
	
	//IPC都是内核级数据结构
    if (shmdt(mStart) == -1)		//对共享操作结束，分离该shmid_ds与该进程关联计数器
	{
		LOG_ERROR(ELOG_KEY, "shmdt(%d) failed", mStart);
    }
	
    if (shmctl(mShmId, IPC_RMID, NULL) == -1)	//删除该shmid_ds共享存储段
	{
		LOG_ERROR(ELOG_KEY, "remove share memory failed: %s", strerror(errno));
    }
	//unlink(mFilename);
}
