
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "wIO.h"

wIO::wIO() :mIOFlag(FLAG_RVSD)
{
	mCreateTime = GetTickCount();
}

wIO::~wIO()
{
	Close();
}

int wIO::SetNonBlock(bool bNonblock)
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
