
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _AGENT_SERVER_TASK_H_
#define _AGENT_SERVER_TASK_H_

#include <arpa/inet.h>
#include <functional>

#include "wType.h"
#include "wLog.h"
#include "wTcpTask.h"
#include "wDispatch.h"
#include "SvrCommand.h"

#define REG_FUNC(ActIdx, vFunc) wDispatch<function<int(char*, int)>, int>::Func_t {ActIdx, std::bind(vFunc, this, std::placeholders::_1, std::placeholders::_2)}
#define DEC_DISP(dispatch) wDispatch<function<int(char*, int)>, int> dispatch
#define DEC_FUNC(func) int func(char *pBuffer, int iLen)

#define AGENT_REG_DISP(cmdid, paraid, func) mDispatch.Register("AgentServerTask", CMD_ID(cmdid, paraid), REG_FUNC(CMD_ID(cmdid, paraid), func));

class AgentServerTask : public wTcpTask
{
	public:
		AgentServerTask();
		AgentServerTask(wSocket *pSocket);
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
		DEC_FUNC(SyncSvrRes);
		DEC_FUNC(ReloadSvrRes);
		DEC_FUNC(InitSvrRes);
	protected:
		DEC_DISP(mDispatch);
};

#endif
