
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include "wChannelTask.h"

wChannelTask(wIO *pIO) : wTask(pIO)
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
