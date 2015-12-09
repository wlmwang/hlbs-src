
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _ROUTER_SERVER_H_
#define _ROUTER_SERVER_H_

#include <arpa/inet.h>

#include "wType.h"
#include "wTimer.h"
#include "wSingleton.h"

#include "wTcpServer.h"
#include "wTcpTask.h"

class RouterServer: public wTcpServer<RouterServer>
{
	public:
		RouterServer();
		virtual ~RouterServer();
		
		virtual void Initialize();
		
		virtual void PrepareRun();
		
		virtual void Run();
		
		virtual wTcpTask* NewTcpTask(wSocket *pSocket);
	    		
		void CheckTimer();
		void CheckTimeout();
		
	private:
		//服务器重连计时器
		wTimer mServerReconnectTimer;

		//客户端检测计时器
		wTimer mClientCheckTimer;
};

#endif
