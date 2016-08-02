
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "RouterServer.h"
#include "RouterServerTask.h"

RouterServer::RouterServer() : wServer<RouterServer>("router服务器")
{
	mConfig = RouterConfig::Instance();
}

wTask* RouterServer::NewTcpTask(wSocket *pSocket)
{
	return new RouterServerTask(pSocket);
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
