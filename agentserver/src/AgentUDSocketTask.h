
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _AGENT_UDSOCKET_TASK_H_
#define _AGENT_UDSOCKET_TASK_H_

#include <arpa/inet.h>
#include <functional>

#include "wCore.h"
#include "wLog.h"
#include "wUDSocketTask.h"
#include "wDispatch.h"
#include "Common.h"
#include "SvrCmd.h"
#include "LoginCmd.h"
#include "AgentConfig.h"
#include "AgentServer.h"

#define AGENT_UNIX_DISP(cmdid, paraid, func) mDispatch.Register("AgentUDSocketTask", CMD_ID(cmdid, paraid), REG_FUNC(CMD_ID(cmdid, paraid), func));

class AgentServer;
class AgentUDSocketTask : public wTcpTask
{
	public:
		AgentUDSocketTask();
		AgentUDSocketTask(wIO *pIO);
		virtual ~AgentUDSocketTask() {}
		void Initialize();
		virtual int HandleRecvMessage(char *pBuffer, int nLen);
		int ParseRecvMessage(struct wCommand *pCommand ,char *pBuffer,int iLen);
		
		DEC_FUNC(GetSvrByGXid);	//获取一个可用svr
		DEC_FUNC(ReportSvr);	//上报
	protected:
		DEC_DISP(mDispatch);
		AgentConfig *mConfig;
		AgentServer *mServer;
};

#endif
