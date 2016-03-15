
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _AGENT_SERVER_TASK_H_
#define _AGENT_SERVER_TASK_H_

#include <arpa/inet.h>
#include <functional>

#include "wCore.h"
#include "wLog.h"
#include "wTcpTask.h"
#include "wDispatch.h"
#include "Common.h"
#include "SvrCmd.h"

#define AGENT_REG_DISP(cmdid, paraid, func) mDispatch.Register("AgentServerTask", CMD_ID(cmdid, paraid), REG_FUNC(CMD_ID(cmdid, paraid), func));

class AgentServerTask : public wTcpTask
{
	public:
		AgentServerTask();
		AgentServerTask(wIO *pIO);
		~AgentServerTask();
		
		void Initialize();
		
		virtual int VerifyConn();
		virtual int Verify();

		virtual int HandleRecvMessage(char * pBuffer, int nLen);
		
		int ParseRecvMessage(struct wCommand* pCommand ,char *pBuffer,int iLen);
		
		DEC_FUNC(ReloadSvrReq);
		DEC_FUNC(GetSvrAll);
		DEC_FUNC(GetSvrByGXid);
		DEC_FUNC(SyncSvrReq);
		
	protected:
		DEC_DISP(mDispatch);
};

#endif
