
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _ROUTER_SERVER_TASK_H_
#define _ROUTER_SERVER_TASK_H_

#include <arpa/inet.h>

#include "wType.h"
#include "wTcpTask.h"
#include "wLog.h"
#include "RtblCommand.h"
/*
#include "wDispatch.h"

#define REG_FUNC(ActName, vFunc) wDispatch<function<int(string, vector<string>)>, int>::Func_t {ActName, std::bind(vFunc, this, std::placeholders::_1, std::placeholders::_2)}
#define DEC_DISP(dispatch) wDispatch<function<int(string, vector<string>)>, int> dispatch
#define DEC_FUNC(func) int func(string sCmd, vector<string>)
*/
class RouterServerTask : public wTcpTask
{
	public:
		RouterServerTask() {}
		~RouterServerTask();
		RouterServerTask(wSocket *pSocket);
		
		virtual int VerifyConn();

		virtual int HandleRecvMessage(char * pBuffer, int nLen);
		
		int ParseRecvMessage(struct wCommand* pCommand, char *pBuffer, int iLen);
};

#endif
