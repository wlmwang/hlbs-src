
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _COMMON_H_
#define _COMMON_H_

#include "wCommand.h"

enum CLIENT_TYPE
{
	CLIENT_USER = 2,
	SERVER_ROUTER,
	SERVER_AGENT,
	SERVER_CMD,
};

#define ROUTER_LOGIN false
#define AGENT_LOGIN false
#define CLIENT_LOGIN false

#define ROUTER_LOCK_FILE "../log/router_hlbs.lock"
#define AGENT_LOCK_FILE "../log/agent_hlbs.lock"

#define ROUTER_PID_FILE "/var/run/hlbs_router.pid"
#define AGENT_PID_FILE "/var/run/hlbs_agent.pid"

#define AGENT_SHM "/tmp/report-agent.bin"

//Dispatch
#define DEC_DISP(dispatch) wDispatch<function<int(char*, int)>, int> dispatch
#define DEC_FUNC(func) int func(char *pBuffer, int iLen)
#define REG_FUNC(ActIdx, vFunc) wDispatch<function<int(char*, int)>, int>::Func_t {ActIdx, std::bind(vFunc, this, std::placeholders::_1, std::placeholders::_2)}
#define REG_DISP(dispatch, classname, cmdid, paraid, func) dispatch.Register(classname, W_CMD(cmdid, paraid), REG_FUNC(W_CMD(cmdid, paraid), func));

#endif
