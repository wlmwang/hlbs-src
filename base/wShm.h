
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

// IPC 共享内存存储
class wShm : private wNoncopyable
{
	public:
		wShm(const char *filename, int pipeid = 'i', size_t size = MSG_QUEUE_LEN);
		~wShm();
		
		char *CreateShm();
		char *AttachShm();
		virtual void RemoveShm();
	
	protected:
		char mFilename[255];
		int mPipeId;
		size_t mSize;
		
		char* mAddr;
		key_t mKey;
		int mShmId;
};

#endif