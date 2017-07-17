
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _AGENT_UNIX_TASK_H_
#define _AGENT_UNIX_TASK_H_

#include "wCore.h"
#include "wMisc.h"
#include "wUnixTask.h"

using namespace hnet;

class AgentUnixTask : public wUnixTask {
public:
	AgentUnixTask(wSocket *socket, int32_t type = 0);

	int GetSvrByGXid(struct Request_t *request);
	int ReportSvr(struct Request_t *request);
};

#endif
