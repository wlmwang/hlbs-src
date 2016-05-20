
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "AgentServer.h"

AgentServer::AgentServer() : wServer<AgentServer>("路由服务器")
{
	mConfig = NULL;
	mInShm = NULL;
	mOutShm = NULL;
	mInMsgQ = NULL;
	mOutMsgQ = NULL;
	mRouterConn = NULL;
	mDetectThread = NULL;
	
	Initialize();
}

AgentServer::~AgentServer() 
{
	SAFE_DELETE(mInShm);
	SAFE_DELETE(mOutShm);
	SAFE_DELETE(mInMsgQ);
	SAFE_DELETE(mOutMsgQ);
	SAFE_DELETE(mRouterConn);
}

void AgentServer::Initialize()
{
	mTicker = GetTickCount();
	mConfig = AgentConfig::Instance();
	mRouterConn = new wMTcpClient<AgentClientTask>();
	mDetectThread = DetectThread::Instance();
	
	InitShm();	//初始化共享内存。与agentcmd进程通信
}

//准备工作
void AgentServer::PrepareRun()
{
	mRouterConn->PrepareStart();
	ConnectRouter(); //连接Router服务
	
	mDetectThread->StartThread(0);
	mRouterConn->StartThread(0);
}

void AgentServer::Run()
{
	CheckQueue();	//读取共享内存
	CheckTimer();	//统计weight结果
}

wTask* AgentServer::NewTcpTask(wIO *pIO)
{
	wTask *pTask = new AgentServerTask(pIO);
	return pTask;
}

void AgentServer::InitShm()
{
	char *pAddr = NULL;

	mInShm = new wShm(AGENT_SHM, 'i', MSG_QUEUE_LEN);
	if((mInShm->CreateShm() != NULL) && ((pAddr = mInShm->AllocShm(MSG_QUEUE_LEN)) != NULL))
	{
		mInMsgQ = new wMsgQueue();
		mInMsgQ->SetBuffer(pAddr, MSG_QUEUE_LEN);
	}
	else
	{
		LOG_ERROR(ELOG_KEY, "[system] Create (In) Share Memory failed");
	}

	mOutShm = new wShm(AGENT_SHM, 'o', MSG_QUEUE_LEN);
	if((mOutShm->CreateShm() != NULL) && ((pAddr = mOutShm->AllocShm(MSG_QUEUE_LEN)) != NULL))
	{
		mOutMsgQ = new wMsgQueue();
		mOutMsgQ->SetBuffer(pAddr, MSG_QUEUE_LEN);
	}
	else
	{
		LOG_ERROR(ELOG_KEY, "[system] Create (Out) Share Memory failed");
	}
}

void AgentServer::ConnectRouter()
{
	AgentConfig::RouterConf_t* pRconf = mConfig->GetOneRouterConf();	//获取一个合法Router服务器配置
	if (pRconf == NULL)
	{
		LOG_ERROR(ELOG_KEY, "[system] Get RouterServer Config failed");
		exit(1);
	}

	//mRouterConn
	bool bRet = mRouterConn->GenerateClient(SERVER_ROUTER, "ROUTER SERVER", pRconf->mIPAddr, pRconf->mPort);
	if (!bRet)
	{
		LOG_ERROR(ELOG_KEY, "[system] Connect to RouterServer failed");
		exit(1);
	}
	LOG_DEBUG(ELOG_KEY, "[system] Connect to RouterServer success, ip(%s) port(%d)", pRconf->mIPAddr, pRconf->mPort);
	
	InitSvrReq();
}

/** 发送初始化svr配置请求 */
int AgentServer::InitSvrReq()
{
	wMTcpClient<AgentClientTask>* pRouterConn = RouterConn();	//客户端连接
	if(pRouterConn == NULL)
	{
		return -1;
	}

	wTcpClient<AgentClientTask>* pClient = pRouterConn->OneTcpClient(SERVER_ROUTER);
	if(pClient != NULL && pClient->TcpTask())
	{
		SvrReqInit_t stSvr;
		return pClient->TcpTask()->SyncSend((char *)&stSvr, sizeof(stSvr));
	}
	return -1;
}

/** 发送重载svr配置请求 */
int AgentServer::ReloadSvrReq()
{
	wMTcpClient<AgentClientTask>* pRouterConn = RouterConn();
	if(pRouterConn == NULL)
	{
		return -1;
	}
	wTcpClient<AgentClientTask>* pClient = pRouterConn->OneTcpClient(SERVER_ROUTER);
	if(pClient != NULL && pClient->TcpTask())
	{
		SvrReqReload_t stSvr;
		return pClient->TcpTask()->SyncSend((char *)&stSvr, sizeof(stSvr));
	}
	return -1;
}

/**
 * 此队列解析不具通用性  Time is up!
 */
void AgentServer::CheckQueue()
{
	int iLen = sizeof(SvrReqReport_t);
	char szBuff[iLen];
	memset(szBuff, 0, sizeof(szBuff));

	int iRet;
	while(true)
	{
		iRet = mInMsgQ->Pop(szBuff, iLen);	//取出数据
		
		//没有消息了
		if(iRet == 0) 
		{
			return;
		}

		//取消息出错
		if(iRet < 0) 
		{
			LOG_ERROR(ELOG_KEY, "[system] get one message from msg queue failed: %d", iRet);
			return;
		}
		//如果消息大小不正确
		if(iRet != iLen) 
		{
			LOG_ERROR(ELOG_KEY, "[system] get a msg with invalid len %d from msg queue", iRet);
			return;
		}
		
		//上报调用结果
		SvrReqReport_t *pReportSvr = (SvrReqReport_t*) szBuff;
		
		if(pReportSvr->mCaller.mCalledGid>0 && pReportSvr->mCaller.mCalledXid>0 && pReportSvr->mCaller.mPort>0 && pReportSvr->mCaller.mHost[0]!=0)
		{
			mConfig->Qos()->CallerNode(pReportSvr->mCaller);
		}
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
}
