
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _AGENT_API_H
#define _AGENT_API_H

#include "wCore.h"
#include "wUDSocket.h"
#include "wTask.h"
#include "Common.h"
#include "Svr.h"
#include "SvrCmd.h"

//#define AGENT_HOST "127.0.0.1"
#define AGENT_PORT 10007
#define AGENT_HOST "/usr/local/webserver/hlbs/agentserver/log/hlbs.sock"

struct postHandle_t
{
	wUDSocket mSock;
	wTask mTask;
	bool mConnecting;
};

extern struct postHandle_t g_handle;

/** 单次获取路由 */
int QueryNode(struct SvrNet_t &stSvr, double iTimeOut, string &sErr);

/** 调用结果上报 */
int NotifyCallerRes(const struct SvrNet_t &stSvr, int iResult, long long iUsetimeUsec, string &sErr);

/** 调用数上报 */
int NotifyCallerNum(const struct SvrNet_t &stSvr, int iReqCount);

int ConnectAgent();
void CloseAgent();

#endif
