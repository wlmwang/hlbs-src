
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "RouterWorker.h"

void RouterWorker::PrepareRun()
{
	mConfig = RouterConfig::Instance();
	if (mConfig == NULL) 
	{
		LOG_ERROR(ELOG_KEY, "[system] RouterConfig instance failed");
		exit(2);
	}
	//重新加载配置（兼容重启时刻）
	mConfig->GetBaseConf();
	mConfig->GetSvrConf();
	mConfig->GetQosConf();

	mServer = RouterServer::Instance();
	if (mServer == NULL) 
	{
		LOG_ERROR(ELOG_KEY, "[system] RouterServer instance failed");
		exit(2);
	}

	mConfig->mProcTitle->Setproctitle("worker process(router)", "HLBS: ");
}

void RouterWorker::Run()
{
	//服务器开始运行
	mServer->WorkerStart(this);
}
