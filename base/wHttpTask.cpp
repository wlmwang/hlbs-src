
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include "wHttpTask.h"

wHttpTask::wHttpTask(wIO *pIO) : wTask(pIO)
{
	Initialize();
}

wHttpTask::wHttpTask()
{
	Initialize();
}

wHttpTask::~wHttpTask() {}

void wHttpTask::Initialize()
{
	//
}
