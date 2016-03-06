
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include "wIO.h"

wIO::wIO() 
{
	Initialize();
}

wIO::~wIO()
{
	Close();
}

void wIO::Initialize()
{
	mFD = FD_UNKNOWN;
	mIOType = TYPE_UNKNOWN;
	mIOFlag = FLAG_UNKNOWN;
	mSockStatus = STATUS_UNKNOWN;
	mSockType = SOCK_UNKNOWN;
	
	mRecvTime = 0;
	mSendTime = 0;
	mCreateTime = GetTickCount();
}

int wSocket::SetNonBlock(bool bNonblock)
{
	if(mFD == FD_UNKNOWN) 
	{
		return -1;
	}
	
	int iFlags = fcntl(mFD, F_GETFL, 0);
	return fcntl(mFD, F_SETFL, (bNonblock == true ? iFlags | O_NONBLOCK : iFlags & ~O_NONBLOCK));
}

void wIO::Close()
{
	if(mFD != FD_UNKNOWN) 
	{
		close(mFD);
	}
	mFD = FD_UNKNOWN;
}

