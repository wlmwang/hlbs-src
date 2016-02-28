
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

//IPC共享内存存储
class wShm : private wNoncopyable
{
	public:
		wShm(const char *filename, int pipeid = 'i', size_t size = MSG_QUEUE_LEN);
		~wShm();
		
		void Initialize();
		char *CreateShm();
		char *AttachShm();
		virtual void RemoveShm();
		
		char *Alloc(size_t len = 0);
		
	protected:
		char mFilename[255];
		int mPipeId;
		size_t mSize;
		
		key_t mKey;
		int mShmId;
		
		char *mStart;	//起始地址
		char *mEnd;		//结束地址
		char *mPos;		//当前处理开始地址
};

#endif