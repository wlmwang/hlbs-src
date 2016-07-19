
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "AgentServer.h"
#include "Common.h"
#include "SvrCmd.h"
#include "AgentServerTask.h"
#include "AgentUnixTask.h"

AgentServer::AgentServer() : wServer<AgentServer>("AGENT服务器")
{
	mConfig = AgentConfig::Instance();
	mDetectThread = DetectThread::Instance();
}

AgentServer::~AgentServer() 
{
	SAFE_DELETE(mRouterConn);
}

//准备工作
void AgentServer::PrepareRun()
{
	ConnectRouter(); //连接Router服务
	mDetectThread != NULL && mDetectThread->StartThread(0);	//探测线程，宕机拉起
	mRouterConn != NULL && mRouterConn->StartThread(0);	//router线程，与router交互
}

void AgentServer::ConnectRouter()
{
	AgentConfig::RouterConf_t* pRconf = mConfig->GetOneRouterConf();	//获取一个合法Router服务器配置
	if (pRconf == NULL)
	{
		LOG_ERROR(ELOG_KEY, "[system] AgentServer get router config failed");
		exit(0);
	}

	//客户端
	mRouterConn = new wMTcpClient<AgentClientTask>();
	mRouterConn->PrepareStart();

	//连接router
	bool bRet = mRouterConn->GenerateClient(SERVER_ROUTER, "ROUTER SERVER", pRconf->mIPAddr, pRconf->mPort);
	if (!bRet)
	{
		LOG_ERROR(ELOG_KEY, "[system] Connect to RouterServer failed");
		exit(0);
	}
	LOG_DEBUG(ELOG_KEY, "[system] Connect to RouterServer success, ip(%s) port(%d)", pRconf->mIPAddr, pRconf->mPort);
	
	//初始化svr配置
	InitSvrReq();
}

wTask* AgentServer::NewTcpTask(wSocket *pSocket)
{
	return new AgentServerTask(pSocket);
}

wTask* AgentServer::NewUnixTask(wSocket *pSocket)
{
	return new AgentUnixTask(pSocket);
}

/** 发送初始化svr配置请求 */
int AgentServer::InitSvrReq()
{
	wMTcpClient<AgentClientTask>* pRouterConn = RouterConn();	//客户端连接
	if (pRouterConn == NULL)
	{
		return -1;
	}

	wTcpClient<AgentClientTask>* pClient = pRouterConn->OneTcpClient(SERVER_ROUTER);
	if (pClient != NULL && pClient->TcpTask())
	{
		struct SvrReqInit_t stSvr;
		return pClient->TcpTask()->SyncSend((char *)&stSvr, sizeof(stSvr));
	}
	return -1;
}

/** 发送重载svr配置请求 */
int AgentServer::ReloadSvrReq()
{
	wMTcpClient<AgentClientTask>* pRouterConn = RouterConn();
	if (pRouterConn == NULL)
	{
		return -1;
	}
	wTcpClient<AgentClientTask>* pClient = pRouterConn->OneTcpClient(SERVER_ROUTER);
	if (pClient != NULL && pClient->TcpTask())
	{
		struct SvrReqReload_t stSvr;
		return pClient->TcpTask()->SyncSend((char *)&stSvr, sizeof(stSvr));
	}
	return -1;
}
