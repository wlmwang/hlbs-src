
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _AGENT_SERVER_H_
#define _AGENT_SERVER_H_

#include <arpa/inet.h>
#include <vector>

#include "wType.h"
#include "wSingleton.h"
#include "wTcpServer.h"
#include "wTcpTask.h"
#include "wMTcpClient.h"
#include "wTcpClient.h"
#include "AgentServerTask.h"
#include "SvrCommand.h"

class AgentServer: public wTcpServer<AgentServer>
{
	public:
		AgentServer();
		virtual ~AgentServer();
		
		virtual void Initialize();
		
		virtual void PrepareRun();
		
		virtual void Run();
		
		virtual wTcpTask* NewTcpTask(wSocket *pSocket);

		void ConnectRouter();
		int InitSvrReq();
		int ReloadSvrReq();
		
		wMTcpClient<AgentServerTask>* RouterConn()
		{
			return mRouterConn;
		}
		
	private:
		wMTcpClient<AgentServerTask> *mRouterConn;	//连接router
};

#endif
