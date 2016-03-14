
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _COMMON_H_
#define _COMMON_H_

//Dispatch
#define REG_FUNC(ActIdx, vFunc) wDispatch<function<int(char*, int)>, int>::Func_t {ActIdx, std::bind(vFunc, this, std::placeholders::_1, std::placeholders::_2)}
#define DEC_DISP(dispatch) wDispatch<function<int(char*, int)>, int> dispatch
#define DEC_FUNC(func) int func(char *pBuffer, int iLen)

enum
{
	CLIENT_USER = 2,
	SERVER_ROUTER,
	SERVER_AGENT,
	SERVER_CMD,
};
#define IPC_SHM "/home/hlfs/disvr/agent_cmd.shm.ipc"

#endif