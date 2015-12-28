
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wMisc.h"
#include "wLog.h"
#include "RouterServer.h"

#include "RouterServerTask.h"

RouterServer::RouterServer():wTcpServer<RouterServer>("路由服务器")
{
	if(mStatus == SERVER_STATUS_INIT)
	{
		Initialize();
	}
}

RouterServer::~RouterServer() 
{
	//...
}

/**
 * 初始化
 */
void RouterServer::Initialize()
{
	//...
}

wTcpTask* RouterServer::NewTcpTask(wSocket *pSocket)
{
	wTcpTask *pTcpTask = new RouterServerTask(pSocket);
	return pTcpTask;
}

//准备工作
void RouterServer::PrepareRun()
{
	//...
}

void RouterServer::Run()
{
	//...
}
