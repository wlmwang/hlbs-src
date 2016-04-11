
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "wChannelTask.h"

wChannelTask::wChannelTask(wIO *pIO) : wTask(pIO)
{
	Initialize();
}

wChannelTask::wChannelTask()
{
	Initialize();
}

wChannelTask::~wChannelTask() {}

void wChannelTask::Initialize()
{
	//
}
