
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include "RouterConfig.h"
#include "RouterServer.h"
#include "RouterServerTask.h"
#include "RouterChannelTask.h"

RouterServer::RouterServer() : wTcpServer<RouterServer>("路由服务器")
{
	Initialize();
}

RouterServer::~RouterServer() 
{
	//...
}

void RouterServer::Initialize()
{
	//...
}

wTask* RouterServer::NewTcpTask(wIO *pIO)
{
	wTask *pTask = new RouterServerTask(pIO);
	return pTask;
}

wTask* RouterServer::NewChannelTask(wIO *pIO)
{
	wTask *pTask = new RouterChannelTask(pIO);
	return pTask;
}

//准备工作
void RouterServer::PrepareRun()
{
	//...
}

void RouterServer::Run()
{
	CheckModSvr();
}

//检测配置文件是否修改
void RouterServer::CheckModSvr()
{
	RouterConfig *pConfig = RouterConfig::Instance();
	if(pConfig->IsModTime())
	{
		SvrResSync_t stSvr;
		stSvr.mCode = 0;

		//读更新配置
		stSvr.mNum = pConfig->GetModSvr(stSvr.mSvr);	//SvrNet_t
		if (stSvr.mNum > 0)
		{
			LOG_DEBUG(ELOG_KEY, "[runtime] broadcast new Svr");
			Broadcast((char *)&stSvr, sizeof(stSvr));	//广播所有agent
		}
	}
}
