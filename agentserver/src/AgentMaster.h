
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _AGENT_MASTER_H_
#define _AGENT_MASTER_H_

#include "wCore.h"
#include "wStatus.h"
#include "wMaster.h"

using namespace hnet;

class AgentMaster : public wMaster {
public:
	AgentMaster(const std::string& title, wServer* server) : wMaster(title, server) { }

	virtual const wStatus& NewWorker(uint32_t slot, wWorker** ptr);
	virtual const wStatus& Reload();
};

#endif
