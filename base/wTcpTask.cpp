
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "wTcpTask.h"

wTcpTask::wTcpTask(wIO *pIO) : wTask(pIO)
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
	mHeartbeatTimes++;
	wCommand vCmd;
	int iRet = SyncSend((char*)&vCmd, sizeof(vCmd));
	return iRet;
}

int wTcpTask::HeartbeatOutTimes()
{
	return mHeartbeatTimes > KEEPALIVE_CNT;
}

int wTcpTask::ClearbeatOutTimes()
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
