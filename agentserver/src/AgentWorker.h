
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _AGENT_WORKER_H_
#define _AGENT_WORKER_H_

#include "wCore.h"
#include "wStatus.h"
#include "wMisc.h"
#include "wWorker.h"

using namespace hnet;

class AgentClient;

class AgentWorker : public wWorker {
public:
	AgentWorker(std::string title, uint32_t slot, wMaster* master);
	virtual ~AgentWorker();

	virtual const wStatus& PrepareRun();

protected:
	AgentClient* mAgentClient;
};

#endif
