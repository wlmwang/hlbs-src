
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
	mClientCheckTimer = wTimer(KEEPALIVE_TIME);
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
	unsigned long long iInterval = (unsigned long long)(GetTickCount() - mLastTicker);

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
	unsigned long long iNowTime = GetTickCount();
	unsigned long long iIntervalTime;
	
	if(mTcpTaskPool.size() > 0)
	{
		vector<wTcpTask*>::iterator iter;
		for(iter = mTcpTaskPool.begin(); iter != mTcpTaskPool.end(); iter++)
		{
			if ((*iter)->Socket()->SocketType() != CONNECT_SOCKET)
			{
				continue;
			}
			iIntervalTime = iNowTime - (*iter)->Socket()->SendTime();	//未接受到消息时间间隔
			//心跳检测
			if(iIntervalTime >= CHECK_CLIENT_TIME)	//3s
			{
				if((*iter)->Heartbeat() < 0 && (*iter)->HeartbeatOutTimes())
				{
					LOG_ERROR("default", "client ip(%s) fd(%d) heartbeat pass limit (3) times, close it", (*iter)->Socket()->IPAddr().c_str(), (*iter)->Socket()->SocketFD());
					if(RemoveEpoll(*iter) >= 0)
					{
						iter = RemoveTaskPool(*iter);
						iter--;
					}
				}
			}
			//超时检测
			/*
			if(iIntervalTime >= CHECK_CLIENT_TIME)	//3s
			{
				LOG_ERROR("default", "client ip(%s) fd(%d) do not send msg and timeout, close it", (*iter)->Socket()->IPAddr().c_str(), (*iter)->Socket()->SocketFD());
				if(RemoveEpoll(*iter) >= 0)
				{
					iter = RemoveTaskPool(*iter);
					iter--;
				}
			}
			*/
		}
	}
}
