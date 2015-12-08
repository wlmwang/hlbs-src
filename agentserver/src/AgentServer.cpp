
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
#include "wFunction.h"
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
	//...
}

/**
 * 初始化
 */
void AgentServer::Initialize()
{
	//初始化定时器
	mServerReconnectTimer = wTimer(RECONNECT_TIME);
	mClientCheckTimer = wTimer(CHECK_CLIENT_TIME);
	
	//连接Router服务
	ConnectRouter();
}

void AgentServer::ConnectRouter()
{
	//mRouterConn
	mRouterConn.GenerateClient(1, "RouterFromAgent", AgentConfig::Instance()->mRouterIPAddr, AgentConfig::Instance()->mRouterPort);
}

wTcpTask* AgentServer::NewTcpTask(wSocket *pSocket)
{
	wTcpTask *pTcpTask = new AgentServerTask(pSocket);
	return pTcpTask;
}

//准备工作
void AgentServer::PrepareRun()
{
}

void AgentServer::Run()
{
	//检查服务器时间
	CheckTimer();
	
	//--------------------------------------------------
	// mConnectCtrl.ReceiveAndProcessMessage(ProcessMessage);
	//-------------------------------------------------- 
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

	//--------------------------------------------------
	//检查连接状态，并试图重连
	// if(mServerReconnectTimer.CheckTimer(iInterval)) 
	// {
	// 	mConnectCtrl.CheckConnectStatus();
	// }
	//-------------------------------------------------- 
	
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
	
	if(!mTcpTaskPool.empty())
	{
		vector<wTcpTask*>::iterator iter;
		for(iter = mTcpTaskPool.begin(); iter != mTcpTaskPool.end(); iter++)
		{
			if((*iter)->Socket()->SocketFlag() == 0)
			{
				continue;
			}
			if ((*iter)->Socket()->SocketType() != CONNECT_SOCKET)
			{
				continue;
			}
			iIntervalTime = iNowTime - (*iter)->Socket()->Stamp();
			if(iIntervalTime >= SOCKET_SEND_TIMEOUT)
			{
				LOG_ERROR("default", "client ip(%s) fd(%d) do not send msg and timeout, close it", (*iter)->Socket()->IPAddr(), (*iter)->Socket()->SocketFD());
				if(RemoveEpoll(*iter))
				{
					RemoveTaskPool(*iter);
				}
			}
		}
	}
}

