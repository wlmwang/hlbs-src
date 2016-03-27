
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include "wHttp.h"

wHttp::wHttp()
{
	Initialize();
}

wHttp::~wHttp() {}

void wHttp::Initialize()
{
	mTaskType = TASK_HTTP;
}

ssize_t wHttp::RecvBytes(char *vArray, size_t vLen)
{
	//
}

ssize_t wHttp::SendBytes(char *vArray, size_t vLen)
{
	//
}
