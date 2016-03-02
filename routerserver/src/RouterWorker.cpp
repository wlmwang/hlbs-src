
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include "RouterWorker.h"

RouterWorker::RouterWorker()
{
	//
}

RouterWorker::~RouterWorker()
{
	//
}

void RouterWorker::Initialize()
{
	mConfig = NULL;
	mServer = NULL;
}

void RouterWorker::PrepareRun()
{
	mConfig = RouterConfig::Instance();
	if(mConfig == NULL) 
	{
		LOG_ERROR(ELOG_KEY, "[startup] Get RouterConfig instance failed");
		exit(1);
	}
	mConfig->mProcTitle->Setproctitle("worker process", "HLFS: ");

	mServer = RouterServer::Instance();
	if(mServer == NULL) 
	{
		LOG_ERROR(ELOG_KEY, "[startup] Get RouterServer instance failed");
		exit(1);
	}
}

void RouterWorker::Run()
{
	//准备工作
	mServer->PrepareStart(mConfig->mIPAddr, mConfig->mPort);

	//服务器开始运行
	LOG_DEBUG(ELOG_KEY, "[startup] RouterServer start succeed");
	
	mServer->Start();
}
