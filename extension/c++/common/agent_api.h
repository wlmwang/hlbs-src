
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _AGENT_API_H
#define _AGENT_API_H

#include "wCore.h"
#include "wStatus.h"
#include "wTask.h"
#include "wTcpSocket.h"
#include "wUdpSocket.h"
#include "wUnixSocket.h"
#include "Svr.h"
#include "SvrCmd.h"

using namespace hnet;

// 0:"UNIX"  1:"TCP" 2:"UDP"
/*
const uint8_t	kAgentSocket	= 0;
const char		kAgentHost[]	= "/var/run/hlbs.sock";
const uint16_t	kAgentPort		= 0;
*/

const uint8_t	kAgentSocket	= 1;
const char		kAgentHost[]	= "127.0.0.1";
const uint16_t	kAgentPort		= 10007;

const uint64_t	kAgentTimeout	= 10;

enum AgentRet_t {
	kOk = 0,
	kUnknown = -1,
	kConnError = -2,	// agent gone
	kSendError = -3,
	kRecvError = -4,
	kDataError = -5		// overload
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

int ConnectAgent();

int CloseAgent();

#endif
