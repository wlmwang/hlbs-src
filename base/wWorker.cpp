
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include "wWorker.h"

void wWorker::Initialize() 
{
	mPid = -1;
	mSlot = -1;
	mExited = -1;
	mData = NULL;
	mRespawn = 0;
}

int wWorker::InitWorker(int type, void *data)
{
	mData = data;
	mPid = getpid();
	mSlot = *(int*)data;
	mExited = 0;
	mRespawn = type;
}

