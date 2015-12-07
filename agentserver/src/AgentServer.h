
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

#include "wTcpClient.h"
#include "wMTcpClient.h"

class AgentServer: public wTcpServer<AgentServer>
{
	public:
		AgentServer();
		virtual ~AgentServer();
		
		virtual void Initialize();
		
		virtual void PrepareRun();
		
		virtual void Run(); //超时&&心跳
		
		virtual wTcpTask* NewTcpTask(wSocket *pSocket);
	    		
		void CheckTimer();
		void CheckTimeout();
		
	private:
	
		wMTcpClient<AgentServer,AgentServerTask> mConn;
		wTcpClient<AgentServerTask> *mClient;
		
		//服务器重连计时器
		wTimer mServerReconnectTimer;

		//客户端检测计时器
		wTimer mClientCheckTimer;
};

#endif
