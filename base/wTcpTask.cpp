
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include "wTcpTask.h"

wTcpTask(wIO *pIO) : wTask(pIO)
{
	Initialize();
}

wTcpTask::wTcpTask()
{
	Initialize();
}

wTcpTask::~wTcpTask() {}

void wTcpTask::Initialize()
{
	mConnType = CLIENT;
	mHeartbeatTimes = 0;
}

int wTcpTask::Heartbeat()
{
	wCommand vCmd;
	SyncSend((char*)&vCmd, sizeof(vCmd));

	mHeartbeatTimes++;
	return -1;
}

int wTcpTask::HeartbeatOutTimes()
{
	return mHeartbeatTimes > 30;
}

int wTcpTask::VerifyConn()
{
	return 0;
}

int wTcpTask::Verify()
{
	return 0;
}
