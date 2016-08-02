
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _AGENT_MASTER_H_
#define _AGENT_MASTER_H_

#include <map>
#include <vector>

#include "wCore.h"
#include "wMaster.h"
#include "AgentConfig.h"
#include "AgentServer.h"

class AgentMaster : public wMaster<AgentMaster>
{
	public:
		virtual ~AgentMaster();
		
		virtual void PrepareRun();
		virtual void Run();
		
	protected:
		char *mTitle {NULL};
		AgentConfig *mConfig {NULL};
		AgentServer *mServer {NULL};
};

#endif
