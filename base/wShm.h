
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_SHM_H_
#define _W_SHM_H_

#include <sys/ipc.h>
#include <sys/shm.h>

#include "wType.h"
#include "wLog.h"
#include "wNoncopyable.h"

/**
 * IPC通信，共享内存管理
 * 注意：此对象本身不再共享内存中，所以需在共享内存上存储本次申请开始地址、结束地址、被分配offset偏移地址。长度为sizeof(struct shmhead_t)
 */
class wShm : private wNoncopyable
{
	struct shmhead_t
	{
		char *mStart;	//开始地址
		char *mEnd;		//结束地址
		char *mUsedOff;	//被分配offset偏移地址，也是下次可使用的开始地址
	};

	public:
		wShm(const char *filename, int pipeid = 'i', size_t size = MSG_QUEUE_LEN);
		void Initialize();
		virtual ~wShm();

		char *CreateShm();
		char *AttachShm();

		char *AllocShm(size_t len = 0);
		void FreeShm();

	protected:
		char mFilename[255];
		int mPipeId;
		key_t mKey;
		int mShmId;
		
		size_t mSize;
		shmhead_t *mShmhead;

		int mPagesize;
};

#endif