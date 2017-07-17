
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _AGENT_TCP_TASK_H_
#define _AGENT_TCP_TASK_H_

#include "wCore.h"
#include "wMisc.h"
#include "wTcpTask.h"

using namespace hnet;

class AgentTcpTask : public wTcpTask {
public:
	AgentTcpTask(wSocket *socket, int32_t type = 0);

	int GetSvrByGXid(struct Request_t *request);
	int ReportSvr(struct Request_t *request);
};

#endif
