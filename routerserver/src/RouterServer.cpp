
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "RouterServer.h"

RouterServer::RouterServer() : wServer<RouterServer>("路由服务器")
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
			Broadcast((char *)&stSvr, sizeof(stSvr));	//广播所有agent
		}
	}
}
