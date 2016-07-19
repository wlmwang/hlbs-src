
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _AGENT_UNIX_TASK_H_
#define _AGENT_UNIX_TASK_H_

#include <arpa/inet.h>
#include <functional>

#include "wCore.h"
#include "wLog.h"
#include "wAssert.h"
#include "wDispatch.h"
#include "wSocket.h"
#include "wUnixTask.h"
#include "AgentConfig.h"
#include "AgentServer.h"

class AgentServer;
class AgentUnixTask : public wUnixTask
{
	public:
		AgentUnixTask();
		AgentUnixTask(wSocket *pSocket);
		void Initialize();

		virtual int HandleRecvMessage(char *pBuffer, int nLen);
		int ParseRecvMessage(struct wCommand *pCommand, char *pBuffer, int iLen);
		
		DEC_FUNC(GetSvrByGXid);	//获取一个可用svr
		DEC_FUNC(ReportSvr);	//上报
	protected:
		DEC_DISP(mDispatch);
		AgentConfig *mConfig {NULL};
		AgentServer *mServer {NULL};
};

#endif
