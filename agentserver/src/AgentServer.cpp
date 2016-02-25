
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

AgentServer::AgentServer():wTcpServer<AgentServer>("路由服务器")
{
	if(mStatus == SERVER_STATUS_INIT)
	{
		mConfig = 0;
		mRouterConn = 0;
		mInShareMem = 0;
		mOutShareMem = 0;
		mInMsgQueue = 0;
		mOutMsgQueue = 0;
		Initialize();
	}
}

AgentServer::~AgentServer() 
{
	SAFE_DELETE(mRouterConn);
	SAFE_DELETE(mInShareMem);
	SAFE_DELETE(mOutShareMem);
	SAFE_DELETE(mInMsgQueue);
	SAFE_DELETE(mOutMsgQueue);
}

/**
 * 初始化
 */
void AgentServer::Initialize()
{
	mTicker = GetTickCount();
	mReportTimer = wTimer(REPORT_TIME_TICK);

	mRouterConn = new wMTcpClient<AgentServerTask>();
	mConfig = AgentConfig::Instance();
	InitShareMemory();
}

void AgentServer::InitShareMemory()
{
	mInShareMem = new wShm(IPC_SHM, 'i', MSG_QUEUE_LEN);
	mOutShareMem = new wShm(IPC_SHM, 'o', MSG_QUEUE_LEN);
	char * pBuff = NULL;
	if((pBuff = mInShareMem->CreateShm()) != NULL)
	{
		mInMsgQueue = new wMsgQueue();
		mInMsgQueue->SetBuffer(pBuff, MSG_QUEUE_LEN);
	}
	else
	{
		LOG_ERROR("error","[startup] Create (In) Share Memory failed");
	}
	if((pBuff = mOutShareMem->CreateShm()) != NULL)
	{
		mOutMsgQueue = new wMsgQueue();
		mOutMsgQueue->SetBuffer(pBuff, MSG_QUEUE_LEN);
	}
	else
	{
		LOG_ERROR("error","[startup] Create (Out) Share Memory failed");
	}
}

wTcpTask* AgentServer::NewTcpTask(wSocket *pSocket)
{
	wTcpTask *pTcpTask = new AgentServerTask(pSocket);
	return pTcpTask;
}

void AgentServer::ConnectRouter()
{
	AgentConfig::RouterConf_t* pRconf = mConfig->GetOneRouterConf();
	if (pRconf == NULL)
	{
		LOG_ERROR("error", "[startup] Get RouterServer Config failed!");
		exit(1);
	}

	//mRouterConn
	bool bRet = mRouterConn->GenerateClient(SERVER_ROUTER, "RouterFromAgent", pRconf->mIPAddr, pRconf->mPort);
	if (!bRet)
	{
		LOG_ERROR("error", "[startup] Connect to RouterServer failed");
		exit(1);
	}

	LOG_ERROR("server", "[connect] Connect to RouterServer success ip(%s) port(%d)", pRconf->mIPAddr, pRconf->mPort);
	//发送初始化svr配置请求
	InitSvrReq();
}

int AgentServer::InitSvrReq()
{
	wMTcpClient<AgentServerTask>* pRouterConn = RouterConn();
	if(pRouterConn == NULL)
	{
		return -1;
	}
	wTcpClient<AgentServerTask>* pClient = pRouterConn->OneTcpClient(SERVER_ROUTER);
	if(pClient != NULL && pClient->TcpTask())
	{
		SvrReqInit_t stSvr;
		return pClient->TcpTask()->SyncSend((char *)&stSvr, sizeof(stSvr));
	}
	return -1;
}

int AgentServer::ReloadSvrReq()
{
	wMTcpClient<AgentServerTask>* pRouterConn = RouterConn();
	if(pRouterConn == NULL)
	{
		return -1;
	}
	wTcpClient<AgentServerTask>* pClient = pRouterConn->OneTcpClient(SERVER_ROUTER);
	if(pClient != NULL && pClient->TcpTask())
	{
		SvrReqReload_t stSvr;
		return pClient->TcpTask()->SyncSend((char *)&stSvr, sizeof(stSvr));
	}
	return -1;
}

//准备工作
void AgentServer::PrepareRun()
{
	mRouterConn->PrepareStart();
	ConnectRouter(); //连接Router服务
}

void AgentServer::Run()
{
	mRouterConn->Start(false);
	CheckQueue();	//读取共享内存
	CheckTimer();	//统计weight结果
}

/**
 * 此队列解析不具通用性  Time is up...
 */
void AgentServer::CheckQueue()
{
	int iLen = sizeof(SvrReportReqId_t);
	char szBuff[iLen];
	memset(szBuff, 0, sizeof(szBuff));

	int iRet;
	while(1)
	{
		iRet = mInMsgQueue->Pop(szBuff, iLen);	//取出数据
		
		//没有消息了
		if(iRet == 0) 
		{
			return;
		}

		//取消息出错
		if(iRet < 0) 
		{
			LOG_ERROR("error", "[runtime] get one message from msg queue failed: %d", iRet);
			return;
		}

		//如果消息大小不正确
		if(iRet != iLen) 
		{
			LOG_ERROR("error", "[runtime] get a msg with invalid len %d from msg queue", iRet);
			return;
		}
		
		SvrReportReqId_t *pReportSvr = (SvrReportReqId_t*) szBuff;
		mConfig->ReportSvr(pReportSvr);
	}
}

void AgentServer::CheckTimer()
{
	unsigned long long iInterval = (unsigned long long)(GetTickCount() - mTicker);

	if(iInterval < 100) 	//100ms
	{
		return;
	}

	//加上间隔时间
	mTicker += iInterval;

	//检测客户端超时
	if(mReportTimer.CheckTimer(iInterval))
	{
		mConfig->Statistics();	//统计上报svr的weight值
	}
}
