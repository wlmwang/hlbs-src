
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _AGENT_SERVER_H_
#define _AGENT_SERVER_H_

#include <arpa/inet.h>
#include <vector>

#include "wCore.h"
#include "wAssert.h"
#include "wSingleton.h"
#include "wTimer.h"
#include "wLog.h"
#include "wMisc.h"
#include "wSocket.h"
#include "wServer.h"
#include "wMTcpClient.h"
#include "AgentClientTask.h"
#include "DetectThread.h"
#include "AgentConfig.h"

class AgentServer: public wServer<AgentServer>
{
	public:
		AgentServer();
		virtual ~AgentServer();

		virtual void PrepareRun();
		virtual wTask* NewTcpTask(wSocket *pSocket);
		virtual wTask* NewUnixTask(wSocket *pSocket);
		
		void ConnectRouter();
		int InitSvrReq();	//连接router成功后，发送初始化svr请求
		int ReloadSvrReq();	//可运行时发送重载svr请求
		wMTcpClient<AgentClientTask>* RouterConn() { return mRouterConn; }

	private:		
		AgentConfig *mConfig {NULL};
		DetectThread *mDetectThread {NULL};
		wMTcpClient<AgentClientTask> *mRouterConn {NULL};	//连接router
};

#endif
