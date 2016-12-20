
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _AGENT_API_H
#define _AGENT_API_H

#include "wCore.h"
#include "wStatus.h"
#include "wTask.h"
#include "wUnixSocket.h"
#include "wTcpSocket.h"
#include "Svr.h"
#include "SvrCmd.h"

using namespace hnet;

// 0:"UNIX"  1:"TCP" 2:"UDP"
const uint8_t	kAgentSocket	= 1;
const char		kAgentHost[]	= "127.0.0.1";
const uint16_t	kAgentPort		= 10007;
const uint64_t	kAgentTimeout	= 30;

enum AgentRet_t {
	kOk = 0,
	kUnknown = -1,
	kConnError = -2,
	kSendError = -3,
	kRecvError = -4,
	kDataError = -5
};

struct postHandle_t {
	wTask *mTask;
	bool mConnecting;
};

extern struct postHandle_t g_handle;

// 单次获取路由
int QueryNode(struct SvrNet_t &svr, double timeout, std::string &err);

// 调用结果上报
int NotifyCallerRes(const struct SvrNet_t &svr, int res, uint64_t usec, std::string &err);

// 调用数上报
int NotifyCallerNum(const struct SvrNet_t &svr, int reqCount);

int ConnectAgent();

int CloseAgent();

#endif
