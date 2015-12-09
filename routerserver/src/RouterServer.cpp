
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include "wLog.h"
#include "RouterServer.h"
#include "wMisc.h"

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
	//初始化定时器
	mServerReconnectTimer = wTimer(RECONNECT_TIME);
	mClientCheckTimer = wTimer(CHECK_CLIENT_TIME);
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
	//检查服务器时间
	CheckTimer();
}

/**
 * 检查服务器时间 试图重连
 */
void RouterServer::CheckTimer()
{
	int iInterval = (int)(GetTickCount() - mLastTicker);

	if(iInterval < 100) 	//100ms
	{
		return;
	}

	//加上间隔时间
	mLastTicker += iInterval;

	//检测客户端超时
	if(mClientCheckTimer.CheckTimer(iInterval))
	{	
		CheckTimeout();
	}
}

/**
 * 检测客户端超时
 */
void RouterServer::CheckTimeout()
{
	int iNowTime = time(NULL);
	int iIntervalTime;
	
	if(mTcpTaskPool.size() > 0)
	{
		vector<wTcpTask*>::iterator iter;
		for(iter = mTcpTaskPool.begin(); iter != mTcpTaskPool.end(); iter++)
		{
			if ((*iter)->Socket()->SocketType() != CONNECT_SOCKET)
			{
				continue;
			}
			iIntervalTime = iNowTime - (*iter)->Socket()->Stamp();
			if(iIntervalTime >= SOCKET_SEND_TIMEOUT)
			{
				LOG_ERROR("default", "client ip(%s) fd(%d) do not send msg and timeout, close it", (*iter)->Socket()->IPAddr().c_str(), (*iter)->Socket()->SocketFD());
				if(RemoveEpoll(*iter) >= 0)
				{
					RemoveTaskPool(*iter);
				}
			}
		}
	}
}
