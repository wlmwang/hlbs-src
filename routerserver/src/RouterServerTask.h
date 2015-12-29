
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _ROUTER_SERVER_TASK_H_
#define _ROUTER_SERVER_TASK_H_

#include <arpa/inet.h>
#include <functional>

#include "wType.h"
#include "wLog.h"
#include "wTcpTask.h"
#include "wDispatch.h"
#include "RtblCommand.h"

#define REG_FUNC(ActIdx, vFunc) wDispatch<function<int(char*, int)>, int>::Func_t {ActIdx, std::bind(vFunc, this, std::placeholders::_1, std::placeholders::_2)}
#define DEC_DISP(dispatch) wDispatch<function<int(char*, int)>, int> dispatch
#define DEC_FUNC(func) int func(char *pBuffer, int iLen)

#define ROUTER_REG_DISP(cmdid, paraid, func) mDispatch.Register("RouterServerTask", ASSERT_CMD(cmdid, paraid), REG_FUNC(ASSERT_CMD(cmdid, paraid), func));

class RouterServerTask : public wTcpTask
{
	public:
		RouterServerTask();
		RouterServerTask(wSocket *pSocket);
		~RouterServerTask();
		
		void Initialize();

		virtual int VerifyConn();
		virtual int Verify();

		virtual int HandleRecvMessage(char * pBuffer, int nLen);
		
		int ParseRecvMessage(struct wCommand* pCommand, char *pBuffer, int iLen);

		DEC_FUNC(ReloadRtblReq);
		DEC_FUNC(InitRtblReq);

	protected:
		DEC_DISP(mDispatch);
};

#endif
