
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wLog.h"
#include "wMisc.h"
#include "AgentServer.h"
#include "AgentConfig.h"
#include "BaseCommand.h"

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
	mRouterConn = new wMTcpClient<AgentServerTask>();
}

wTcpTask* AgentServer::NewTcpTask(wSocket *pSocket)
{
	wTcpTask *pTcpTask = new AgentServerTask(pSocket);
	return pTcpTask;
}

void AgentServer::ConnectRouter()
{
	AgentConfig *pConfig = AgentConfig::Instance();
	
	//mRouterConn
	mRouterConn->GenerateClient(ROUTER_SERVER_TYPE, "RouterFromAgent", pConfig->mRouterIPAddr, pConfig->mRouterPort);
	
	//发送初始化rtbl配置请求
	InitRtblReq();
}

int AgentServer::InitRtblReq()
{
	wMTcpClient<AgentServerTask>* pRouterConn = RouterConn();
	if(pRouterConn == NULL)
	{
		return -1;
	}
	wTcpClient<AgentServerTask>* pClient = pRouterConn->OneTcpClient(ROUTER_SERVER_TYPE);
	if(pClient != NULL && pClient->TcpTask())
	{
		RtblReqInit_t vRtl;
		return pClient->TcpTask()->SyncSend((char *)&vRtl, sizeof(vRtl));
	}
	return -1;
}

int AgentServer::ReloadRtblReq()
{
	wMTcpClient<AgentServerTask>* pRouterConn = RouterConn();
	if(pRouterConn == NULL)
	{
		return -1;
	}
	wTcpClient<AgentServerTask>* pClient = pRouterConn->OneTcpClient(ROUTER_SERVER_TYPE);
	if(pClient != NULL && pClient->TcpTask())
	{
		RtblReqReload_t vRtl;
		return pClient->TcpTask()->SyncSend((char *)&vRtl, sizeof(vRtl));
	}
	return -1;
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
void AgentServer::CheckTimeout()
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
			if((*iter)->Socket()->ConnType() == SERVER_CMD)	//信任客户端，不需要心跳
			{
				continue;
			}
			iIntervalTime = iNowTime - (*iter)->Socket()->SendTime();
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

