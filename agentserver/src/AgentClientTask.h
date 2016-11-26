
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

	int InitSvrRes(struct Request_t *request);
	int ReloadSvrRes(struct Request_t *request);
	int SyncSvrRes(struct Request_t *request);
};

#endif
