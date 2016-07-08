
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
#include "wTcpTask.h"
#include "wDispatch.h"
#include "Common.h"
#include "SvrCmd.h"

#define AGENTCLT_REG_DISP(cmdid, paraid, func) mDispatch.Register("AgentClientTask", CMD_ID(cmdid, paraid), REG_FUNC(CMD_ID(cmdid, paraid), func));

class AgentClientTask : public wTcpTask
{
	public:
		AgentClientTask();
		AgentClientTask(wIO *pIO);
		virtual ~AgentClientTask() {}
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
