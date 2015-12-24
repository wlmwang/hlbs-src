
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
#include "wDispatch.h"
#include "RtblCommand.h"

#define REG_FUNC_a(ActIdx, vFunc) wDispatch<function<int(char*, int)>, int>::Func_t {ActIdx, std::bind(vFunc, this, std::placeholders::_1, std::placeholders::_2)}
#define DEC_DISP_a(dispatch) wDispatch<function<int(char*, int)>, int> dispatch
#define DEC_FUNC_a(func) int func(char *pBuffer, int iLen)

class AgentCmdTask : public wTcpTask
{
	public:
		AgentCmdTask();
		AgentCmdTask(wSocket *pSocket);
		~AgentCmdTask();
		
		void Initialize();
		
		virtual int Verify();
		
		virtual int ConnType()
		{
			return SERVER_CMD;
		}
		
		virtual int HandleRecvMessage(char * pBuffer, int nLen);
		
		int ParseRecvMessage(struct wCommand* pCommand ,char *pBuffer,int iLen);
		
		DEC_FUNC_a(RtblResData);
		DEC_FUNC_a(RtblUpdateResData);

	protected:
		DEC_DISP_a(mDispatch);
};

#endif
