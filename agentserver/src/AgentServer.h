
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _AGENT_SERVER_H_
#define _AGENT_SERVER_H_

#include <arpa/inet.h>
#include <vector>

#include "wType.h"
#include "wTimer.h"
#include "wSingleton.h"

#include "wTcpServer.h"
#include "wTcpTask.h"

#include "AgentServerTask.h"
#include "wMTcpClient.h"
#include "wTcpClient.h"

class AgentServer: public wTcpServer<AgentServer>
{
	public:
		AgentServer();
		virtual ~AgentServer();
		
		virtual void Initialize();
		
		virtual void PrepareRun();
		
		virtual void Run(); //超时&&心跳
		
		virtual wTcpTask* NewTcpTask(wSocket *pSocket);
	    
		void ConnectRouter();
		void CheckTimer();
		void CheckTimeout();
		
	private:
		wMTcpClient<AgentServer,AgentServerTask> mRouterConn;	//连接router
		
		//服务器重连计时器
		wTimer mServerReconnectTimer;

		//客户端检测计时器
		wTimer mClientCheckTimer;
};

#endif
