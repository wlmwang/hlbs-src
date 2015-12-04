
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
#include "RouterClient.h"
#include "wMisc.h"

#include "RouterClientTask.h"

RouterClient::RouterClient():wTcpClient<RouterClient , 100*1024 , 100*1024>()
{
	//if(mStatus == SERVER_STATUS_INIT)
	{
		Initialize();
	}
	
	//初始化buf
	mInMsgQueue.SetBuffer(mInBufferPtr, MSG_QUEUE_LEN);
	mOutMsgQueue.SetBuffer(mOutBufferPtr, MSG_QUEUE_LEN);
}

RouterClient::~RouterClient() 
{
	//...
}

/**
 * 初始化
 */
void RouterClient::Initialize()
{	
	//初始化定时器
	mServerReconnectTimer = wTimer(RECONNECT_TIME);
	mClientCheckTimer = wTimer(CHECK_CLIENT_TIME);
	
	//初始化消息队列
	InitailizeBuffer();
}

/**
 * 初始化消息队列
 * 队列多进程通信
 */
void RouterClient::InitailizeBuffer()
{
	//如果没有router_pipe就先创建一个
	system("touch router_pipe");

	const char *pFilename = "./router_pipe";
	mInBufferPtr = CreateShareMemory(pFilename, 'i', MSG_QUEUE_LEN);
	mOutBufferPtr = CreateShareMemory(pFilename, 'o', MSG_QUEUE_LEN);
	if(mInBufferPtr == NULL || mOutBufferPtr == NULL)
	{
		printf("Create share memory for msg queue failed\n");
		exit(1);
	}
}

wTcpTask* RouterClient::NewTcpTask(int iNewSocket, struct sockaddr_in *stSockAddr)
{
	wTcpTask *pTcpTask = new RouterClientTask(iNewSocket, stSockAddr);
	return pTcpTask;
}

//准备工作
void RouterClient::PrepareRun()
{
	//--------------------------------------------------
	// mConnectCtrl.ListenToAddress(LISTEN_TO_ALL, RouterConfig::GetSingletonPtr()->mInIPAddr, (short)RouterConfig::GetSingletonPtr()->mInPort);
	//-------------------------------------------------- 
}

//主循环
void RouterClient::Run()
{
	//检查服务器时间
	CheckTimer();
	
	//处理接受消息
	ProcessRecvMessage();
	
	//--------------------------------------------------
	// mConnectCtrl.ReceiveAndProcessMessage(ProcessMessage);
	//-------------------------------------------------- 
}

void RouterClient::ProcessRecvMessage()
{
    /*
	if(!mTcpTaskPool.empty())
	{
		vector<wTcpTask*>::iterator iter;
		for(iter = mTcpTaskPool.begin(); iter != mTcpTaskPool.end(); iter++)
		{
			(*iter)->ProcessRecvMessage();
		}
	}
    */
}

//检查服务器时间
void RouterClient::CheckTimer()
{
    /*
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
		CheckClientTimeout();
	}
    */
}

/**
 * 检测客户端超时
 */
void RouterClient::CheckClientTimeout()
{
	int iNowTime = time(NULL);
	int iIntervalTime;
    /*	
	if(!mTcpTaskPool.empty())
	{
		vector<wTcpTask*>::iterator iter;
		for(iter = mTcpTaskPool.begin(); iter != mTcpTaskPool.end(); iter++)
		{
			if((*iter)->mSocketFlag == 0)
			{
				continue;
			}
			if ((*iter)->mSocketType != CONNECT_SOCKET)
			{
				continue;
			}
			iIntervalTime = iNowTime - (*iter)->mStamp;
			if(iIntervalTime >= SOCKET_SEND_TIMEOUT)
			{
				LOG_ERROR("default", "client ip(%s) fd(%d) do not send msg and timeout, close it", (*iter)->mIPAddr, (*iter)->mSocketFD);
				if (RemoveEpoll(*iter))
				{
					RemoveTaskPool(*iter);
					//*iter->Terminate(CLOSE_REASON vReason);
				}
			}
		}
	}
    */
}
