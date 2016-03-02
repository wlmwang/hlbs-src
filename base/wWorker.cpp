
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
	mData = NULL;
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

void wWorker::PrepareRun()
{
	mCh.Close();
}

void wWorker::Run()
{
	mCh.Close();
}

int wWorker::PrepareStart(int type, void *data) 
{
	PrepareRun();

	mPid = getpid();
	mSlot = *(int *) data;
	mExited = 0;
	mRespawn = type;

	//setproctitle("worker process"); //设置进程名称
}

int wWorker::Start() 
{
	while(true)
	{
		Run();
	}

	//进程退出
	exit(0);
}

