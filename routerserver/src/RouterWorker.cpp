
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

	//重新加载配置（兼容重启时刻）
	pConfig->GetBaseConf();
	pConfig->GetSvrConf();
	pConfig->GetQosConf();

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
	LOG_DEBUG(ELOG_KEY, "[startup] RouterServer start succeed");
	
	mServer->WorkerStart(this);
}
