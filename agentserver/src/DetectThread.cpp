
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include "DetectThread.h"

DetectThread::DetectThread(char *pPrompt, int iLen)
{
	//
}

DetectThread::~DetectThread()
{
	//
}

bool DetectThread::IsBlocked()
{
	return false;
}

int DetectThread::PrepareRun()
{
	mDeteMutex = new wMutex();
	
	unsigned int nIp = GetIpByIF("eth1");
	if(!nIp)
	{
		nIp = GetIpByIF("eth0");
	}
	if(nIp)
	{
		mLocalIp = nIp;
	}
	return 0;
}

int DetectThread::Run()
{
	char sIp[32] = {0};
	inet_ntop(AF_INET, &mLocalIp, sIp, sizeof(sIp));
	
	while (true)
	{
		mDeteMutex->Lock();
		//
	}
	return 0;
}
