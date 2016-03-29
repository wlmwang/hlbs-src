
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include "RouterServer.h"

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
	mConfig = RouterConfig::Instance();
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

//检测配置文件是否修改(增量同步)
void RouterServer::CheckModSvr()
{
	if(mConfig->IsModTime())
	{
		struct SvrResSync_t stSvr;
		stSvr.mCode = 0;

		stSvr.mNum = mConfig->GetModSvr(stSvr.mSvr);	//SvrNet_t
		if (stSvr.mNum > 0)
		{
			LOG_DEBUG(ELOG_KEY, "[runtime] sync new or change(version) svr gid=%d xid=%d host=%s port=%d weight=%d ver=%d",
				stSvr.mGid,stSvr.mXid,stSvr.mHost,stSvr.mPort,stSvr.mWeight,stSvr.mVersion);

			Broadcast((char *)&stSvr, sizeof(stSvr));	//广播所有agent
		}
	}
}
