
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _AGENT_CLIENT_TASK_H_
#define _AGENT_CLIENT_TASK_H_

#include <arpa/inet.h>
#include <functional>

#include "wCore.h"
#include "wAssert.h"
#include "wLog.h"
#include "wDispatch.h"
#include "wSocket.h"
#include "wTcpTask.h"

class AgentClientTask : public wTcpTask
{
	public:
		AgentClientTask();
		AgentClientTask(wSocket *pSocket);
		void Initialize();

		virtual int VerifyConn();
		virtual int Verify();
		virtual int HandleRecvMessage(char *pBuffer, int nLen);
		int ParseRecvMessage(struct wCommand *pCommand ,char *pBuffer,int iLen);
			
		DEC_FUNC(SyncSvrRes);
		DEC_FUNC(ReloadSvrRes);
		DEC_FUNC(InitSvrRes);
	protected:
		DEC_DISP(mDispatch);
};

#endif
