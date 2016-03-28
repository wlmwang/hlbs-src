
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _AGENT_API_H
#define _AGENT_API_H

#include "wCore.h"
#include "wSocket.h"
#include "wTask.h"
#include "Common.h"
#include "Svr.h"
#include "SvrCmd.h"

#define AGENT_HOST "192.168.8.13"
#define AGENT_PORT 10007

struct postHandle_t
{
	wSocket *mSock;
	wTask *mTask;
};

extern struct postHandle_t g_handle;

/** 单次获取路由 */
int QueryNode(struct SvrNet_t &stSvr, double iTimeOut, string &sErr);

/** 调用结果上报 */
int NotifyCallerRes(const struct SvrNet_t &stSvr, int iResult, long long iUsetimeUsec, string &sErr);

/** 调用数上报 */
int NotifyCallerNum(const struct SvrNet_t &stSvr, int iReqCount);

int ConnectAgent(struct postHandle_t *pHandle);
void Release(struct postHandle_t *pHandle);

#endif
