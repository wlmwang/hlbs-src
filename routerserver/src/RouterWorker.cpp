
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include "RouterWorker.h"

RouterWorker::RouterWorker(int iSlot) : wWorker(iSlot)
{
	Initialize();
}

RouterWorker::~RouterWorker()
{
	SAFE_DELETE(mConfig);
    SAFE_DELETE(mServer);
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
	//mConfig->mProcTitle->InitSetproctitle();
	
	pConfig->GetBaseConf();
	pConfig->GetSvrConf();

	mServer = RouterServer::Instance();
	if(mServer == NULL) 
	{
		LOG_ERROR(ELOG_KEY, "[startup] Get RouterServer instance failed");
		exit(1);
	}

	mConfig->mProcTitle->Setproctitle("worker process", "HLFS: ");
}

void RouterWorker::Run()
{
	//服务器开始运行
	//LOG_DEBUG(ELOG_KEY, "[startup] RouterServer start succeed");
	
	mServer->WorkerStart(this);
}
