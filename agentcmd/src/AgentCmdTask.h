
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _AGENT_SERVER_TASK_H_
#define _AGENT_SERVER_TASK_H_

#include <arpa/inet.h>
#include <functional>

#include "wType.h"
#include "wTcpTask.h"
#include "wLog.h"
#include "RtblCommand.h"
#include "wCommand.h"

#include "wDispatch.h"

#define REG_FUNC(ActIdx, vFunc) wDispatch<function<int(char*, int)>, int>::Func_t {ActIdx, std::bind(vFunc, this, std::placeholders::_1, std::placeholders::_2)}
#define DEC_DISP(dispatch) wDispatch<function<int(char*, int)>, int> dispatch
#define DEC_FUNC(func) int func(char *pBuffer, int iLen)

class AgentCmdTask : public wTcpTask
{
	public:
		AgentCmdTask();
		AgentCmdTask(wSocket *pSocket);
		~AgentCmdTask();
		
		void Initialize();
		
		virtual int Verify();
		
		virtual int HandleRecvMessage(char * pBuffer, int nLen);
		
		int ParseRecvMessage(struct wCommand* pCommand ,char *pBuffer,int iLen);
		
		DEC_FUNC(RtblResData);
		DEC_FUNC(RtblSetResData);
	protected:
		DEC_DISP(mDispatch);
};

#endif
