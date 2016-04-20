
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "wCore.h"
#include "wLog.h"
#include "AgentConfig.h"
#include "AgentMaster.h"

int main(int argc, const char *argv[])
{
	//config
	AgentConfig *pConfig = AgentConfig::Instance();
	if(pConfig == NULL) 
	{
		cout << "[startup] Get AgentConfig instance failed" << endl;
		exit(1);
	}
	if (pConfig->GetOption(argc, argv) < 0)
	{
		cout << "[startup] Get Option failed" << endl;
		exit(1);
	}
	pConfig->GetBaseConf();
	pConfig->GetRouterConf();
	pConfig->GetQosConf();
	
	//daemon
	if (pConfig->mDaemon == 1)
	{
		if (InitDaemon("../log/hlbs.lock") < 0)
		{
			LOG_ERROR(ELOG_KEY, "[startup] Create daemon failed!");
			exit(1);
		}
	}

	//master
	AgentMaster *pMaster = AgentMaster::Instance();
	if(pMaster == NULL) 
	{
		LOG_ERROR(ELOG_KEY, "[startup] Get AgentMaster instance failed");
		exit(1);
	}

	pMaster->PrepareStart();
	pMaster->SingleStart();

	LOG_SHUTDOWN_ALL;
	return 0;
}
