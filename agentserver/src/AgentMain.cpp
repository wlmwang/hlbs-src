
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "wCore.h"
#include "wLog.h"
#include "Command.h"	//pid file
#include "AgentConfig.h"
#include "AgentMaster.h"

int main(int argc, const char *argv[])
{
	//config
	AgentConfig *pConfig = AgentConfig::Instance();
	if(pConfig == NULL) 
	{
		cout << "[system] AgentConfig instance failed" << endl;
		exit(0);
	}
	if (pConfig->GetOption(argc, argv) < 0)
	{
		cout << "[system] Command line Option failed" << endl;
		exit(0);
	}
	
	//daemon
	if (pConfig->mDaemon == 1)
	{
		if (InitDaemon(AGENT_LOCK_FILE) < 0)
		{
			LOG_ERROR(ELOG_KEY, "[system] Create daemon failed!");
			exit(0);
		}
	}

	//Init config
	pConfig->GetBaseConf();
	pConfig->GetRouterConf();
	pConfig->GetQosConf();

	//master && server
	AgentMaster *pMaster = AgentMaster::Instance();
	if(pMaster == NULL) 
	{
		LOG_ERROR(ELOG_KEY, "[system] AgentMaster instance failed");
		exit(0);
	}
	pMaster->PrepareStart();
	pMaster->SingleStart();

	LOG_SHUTDOWN_ALL;
	return 0;
}
