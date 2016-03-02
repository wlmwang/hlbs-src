
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include "RouterWorker.h"

RouterWorker::RouterWorker()
{
	//
}

RouterWorker:~RouterWorker()
{
	//
}

void RouterWorker::Initialize()
{
	//
}

void RouterWorker::PrepareRun()
{
	RouterConfig *pConfig = RouterConfig::Instance();

	pConfig->mProcTitle->Setproctitle("worker process", "HLFS: ");
}

void RouterWorker::Run()
{
	RouterConfig *pConfig = RouterConfig::Instance();

	//获取服务器实体
    RouterServer *pServer = RouterServer::Instance();
	if(pServer == NULL) 
	{
		LOG_ERROR(ELOG_KEY, "[startup] Get RouterServer instance failed");
		exit(1);
	}
	
	//准备工作
	pServer->PrepareStart(pConfig->mIPAddr, pConfig->mPort);

	//服务器开始运行
	LOG_INFO(ELOG_KEY, "[startup] RouterServer start succeed");
	pServer->Start();
}
