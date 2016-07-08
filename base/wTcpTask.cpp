
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "wTcpTask.h"

wTcpTask::wTcpTask(wIO *pIO) : wTask(pIO)
{
	mHeartbeatTimes = 0;
}

wTcpTask::wTcpTask()
{
	mHeartbeatTimes = 0;
}

int wTcpTask::VerifyConn()
{
	return 0;
}

int wTcpTask::Verify()
{
	return 0;
}
