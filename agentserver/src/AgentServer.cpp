
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "AgentServer.h"

AgentServer::AgentServer() : wServer<AgentServer>("AGENT服务器")
{
	Initialize();
}

AgentServer::~AgentServer() 
{
	SAFE_DELETE(mRouterConn);
}

void AgentServer::Initialize()
{
	mConfig = AgentConfig::Instance();
	mDetectThread = DetectThread::Instance();
	ConnectRouter(); //连接Router服务
}

void AgentServer::ConnectRouter()
{
	AgentConfig::RouterConf_t* pRconf = mConfig->GetOneRouterConf();	//获取一个合法Router服务器配置
	if (pRconf == NULL)
	{
		LOG_ERROR(ELOG_KEY, "[system] Get RouterServer Config failed");
		exit(0);
	}
	
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

//准备工作
void AgentServer::PrepareRun()
{
	mDetectThread != NULL && mDetectThread->StartThread(0);	//探测线程，宕机拉起
	mRouterConn != NULL && mRouterConn->StartThread(0);	//router线程，与router交互

	//创建Unix Domain Socket监听请求 （临时方案）
	mUDListenSock = new wUDSocket();
	if (mUDListenSock == NULL)
	{
		LOG_ERROR(ELOG_KEY, "[system] new unix socket failed");
		exit(0);
	}

	int iFD = mUDListenSock->Open();
	if (iFD == -1)
	{
		LOG_ERROR(ELOG_KEY, "[system] listen unix socket open failed");
		SAFE_DELETE(mUDListenSock);
		exit(0);
	}
	
	//listen socket
	if(mUDListenSock->Listen("../log/hlbs.sock") < 0)
	{
		mErr = errno;
		LOG_ERROR(ELOG_KEY, "[system] listen unix socket failed: %s", strerror(mErr));
		SAFE_DELETE(mUDListenSock);
		exit(0);
	}
	
	//nonblock
	if(mUDListenSock->SetNonBlock() < 0) 
	{
		LOG_ERROR(ELOG_KEY, "[system] Set unix socket non block failed: %d, close it", iFD);
		SAFE_DELETE(mUDListenSock);
		exit(0);
	}
	mUDListenSock->SockStatus() = STATUS_LISTEN;

	mTask = NewUDSocketTask(mUDListenSock);
	if(NULL != mTask)
	{
		mTask->Status() = TASK_RUNNING;
		if (AddToEpoll(mTask) >= 0)
		{
			AddToTaskPool(mTask);
		}
		else
		{
			mTask->DeleteIO();
			SAFE_DELETE(mTask);
			exit(0);
		}
	}
}

void AgentServer::Run() 
{
	//...
}

wTask* AgentServer::NewTcpTask(wIO *pIO)
{
	wTask *pTask = new AgentServerTask(pIO);
	return pTask;
}

wTask* AgentServer::NewUDSocketTask(wIO *pIO)
{
	wTask *pTask = new AgentUDSocketTask(pIO);
	return pTask;
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
