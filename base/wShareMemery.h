
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_SOCKET_H_
#define _W_SOCKET_H_

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

class wShareMemery : private wNoncopyable
{
	public:
		wShareMemery(const char *filename, int pipeid = 'i', size_t size = MSG_QUEUE_LEN);
		~wShareMemery();
		
		char *CreateShareMemory();
		char *AttachShareMemory();
		virtual void RemoveShareMemory();
	
	protected:
	
		char* mFilename;
		int mPipeId;
		size_t mSize;
		
		char* mAddr;
		key_t mKey;
		int mShmId;
};

#endif