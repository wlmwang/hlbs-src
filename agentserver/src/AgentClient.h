
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _AGENT_CLIENT_H_
#define _AGENT_CLIENT_H_

#include "wCore.h"
#include "wStatus.h"
#include "wMultiClient.h"

using namespace hnet;

const int kType = 0;

class AgentClient : public wMultiClient {
public:
	AgentClient(wConfig* config, wServer* server = NULL) : wMultiClient(config, server) { }

	virtual const wStatus& PrepareRun();
	virtual const wStatus& NewTcpTask(wSocket* sock, wTask** ptr, int type = 0);

	// 发送初始化svr配置请求
	const wStatus& InitSvrReq();

	// 发送重载svr配置请求
	const wStatus& ReloadSvrReq();
};

#endif
