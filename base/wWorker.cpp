
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include "wWorker.h"

wWorker::wWorker() 
{
	Initialize();
}

wWorker::~wWorker() {}

void wWorker::Initialize() 
{
	mPid = -1;
	mSlot = -1;
	mExited = -1;
	mRespawn = 0;
}

int wWorker::InitChannel()
{
	return mCh.Open();
}

void wWorker::Close()
{
	mCh.Close();
}

void wWorker::PrepareRun() {}

void wWorker::Run() {}

void wWorker::PrepareStart(int type, void *data) 
{
	PrepareRun();

	mPid = getpid();
	mSlot = *(int *) data;
	mExited = 0;
	mRespawn = type;
}

void wWorker::Start() 
{
	while(true)
	{
		Run();
	}

	//进程退出
	exit(0);
}

