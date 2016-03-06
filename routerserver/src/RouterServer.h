
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _ROUTER_SERVER_H_
#define _ROUTER_SERVER_H_

#include <arpa/inet.h>

#include "wType.h"
#include "wMisc.h"
#include "wLog.h"
#include "wSingleton.h"
#include "wTask.h"
#include "wIO.h"
#include "wTcpServer.h"

class RouterServer: public wTcpServer<RouterServer>
{
	public:
		RouterServer();
		virtual ~RouterServer();
		
		void Initialize();
		
		virtual void PrepareRun();
		
		virtual void Run();
		
		virtual wTask* NewTcpTask(wIO *pIO);
		virtual wTask* NewChannelTask(wIO *pIO);

		void CheckModSvr();	//检测配置文件是否修改
};

#endif
