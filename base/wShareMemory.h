
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_SHARE_MEMORY_H_
#define _W_SHARE_MEMORY_H_

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "wType.h"
#include "wLog.h"
#include "wNoncopyable.h"

class wShareMemory : private wNoncopyable
{
	public:
		wShareMemory(const char *filename, int pipeid = 'i', size_t size = MSG_QUEUE_LEN);
		~wShareMemory();
		
		char *CreateShareMemory();
		char *AttachShareMemory();
		virtual void RemoveShareMemory();
	
	protected:
	
		char mFilename[64];
		int mPipeId;
		size_t mSize;
		
		char* mAddr;
		key_t mKey;
		int mShmId;
};

#endif