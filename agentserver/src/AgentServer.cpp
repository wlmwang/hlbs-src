
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
#include "wMisc.h"

#include "AgentServer.h"
#include "AgentConfig.h"

AgentServer::AgentServer():wTcpServer<AgentServer>("路由服务器")
{
	if(mStatus == SERVER_STATUS_INIT)
	{
		Initialize();
	}
}

AgentServer::~AgentServer() 
{
	SAFE_DELETE(mRouterConn);
}

/**
 * 初始化
 */
void AgentServer::Initialize()
{
	//初始化定时器
	mClientCheckTimer = wTimer(KEEPALIVE_TIME);
	mRouterConn = wMTcpClient<AgentServer,AgentServerTask>::Instance();
}

wTcpTask* AgentServer::NewTcpTask(wSocket *pSocket)
{
	wTcpTask *pTcpTask = new AgentServerTask(pSocket);
	return pTcpTask;
}

void AgentServer::ConnectRouter()
{
	//mRouterConn
	mRouterConn->GenerateClient(1, "RouterFromAgent", AgentConfig::Instance()->mRouterIPAddr, AgentConfig::Instance()->mRouterPort);
}


//准备工作
void AgentServer::PrepareRun()
{
	mRouterConn->PrepareStart();
	
	//连接Router服务
	ConnectRouter();
}

void AgentServer::Run()
{
	mRouterConn->Start(false);

	//检查服务器时间
	CheckTimer();
}

//检查服务器时间
void AgentServer::CheckTimer()
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
void AgentServer::CheckTimeout()
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
			//心跳检测
			if(iIntervalTime >= KEEPALIVE_TIME)	//1s
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

