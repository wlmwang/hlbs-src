
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _ROUTER_SERVER_TASK_H_
#define _ROUTER_SERVER_TASK_H_

#include <arpa/inet.h>
#include <functional>

#include "wCore.h"
#include "wLog.h"
#include "wIO.h"
#include "wTcpTask.h"
#include "wTask.h"
#include "wDispatch.h"
#include "SvrCmd.h"
#include "LoginCmd.h"
#include "wAssert.h"
#include "RouterConfig.h"
#include "RouterServer.h"

class RouterServer;
class RouterServerTask : public wTcpTask
{
	public:
		RouterServerTask();
		RouterServerTask(wIO *pIO);
		virtual ~RouterServerTask() {}
		void Initialize();

		virtual int VerifyConn();
		virtual int Verify();
		virtual int HandleRecvMessage(char *pBuffer, int nLen);
		int ParseRecvMessage(struct wCommand* pCommand, char *pBuffer, int iLen);

		DEC_FUNC(InitSvrReq);
		DEC_FUNC(ReloadSvrReq);
		
	protected:
		DEC_DISP(mDispatch);
		RouterConfig *mConfig;
		RouterServer *mServer;
};

#endif
