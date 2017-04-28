
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _AGENT_CLIENT_TASK_H_
#define _AGENT_CLIENT_TASK_H_

#include "wCore.h"
#include "wStatus.h"
#include "wMisc.h"
#include "wTcpTask.h"

using namespace hnet;

class AgentClientTask : public wTcpTask {
public:
	AgentClientTask(wSocket *socket, int32_t type);

    virtual const wStatus& Connect();
    virtual const wStatus& ReConnect();

	const wStatus& InitSvrReq();	// 发送初始化svr配置请求
	const wStatus& ReloadSvrReq();	// 发送重载svr配置请求

	int InitSvrRes(struct Request_t *request);
	int ReloadSvrRes(struct Request_t *request);
	int SyncSvrRes(struct Request_t *request);
};

#endif
