
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "wUDSocketTask.h"

wUDSocketTask::wUDSocketTask(wIO *pIO) : wTask(pIO)
{
	Initialize();
}

wUDSocketTask::wUDSocketTask()
{
	Initialize();
}

wUDSocketTask::~wUDSocketTask() {}

void wUDSocketTask::Initialize()
{
	mConnType = CLIENT;
	mHeartbeatTimes = 0;
}

int wUDSocketTask::VerifyConn()
{
	return 0;
}

int wUDSocketTask::Verify()
{
	return 0;
}
