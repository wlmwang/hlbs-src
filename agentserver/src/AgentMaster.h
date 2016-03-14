
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
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
		AgentMaster();
		virtual ~AgentMaster();
		void Initialize();
		
		virtual void PrepareRun();
		virtual void Run();
	
	protected:
		char *mTitle;

		AgentConfig *mConfig;
		AgentServer *mServer;
};

#endif
