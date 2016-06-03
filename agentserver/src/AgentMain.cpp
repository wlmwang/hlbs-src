
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "wCore.h"
#include "wLog.h"
#include "AgentConfig.h"
#include "AgentMaster.h"

void ServerExit()
{
	//code...
}

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
		if (InitDaemon("../log/hlbs.lock") < 0)
		{
			LOG_ERROR(ELOG_KEY, "[system] Create daemon failed!");
			exit(0);
		}
	}

	//init config
	pConfig->GetBaseConf();
	pConfig->GetRouterConf();
	pConfig->GetQosConf();

	//master
	AgentMaster *pMaster = AgentMaster::Instance();
	if(pMaster == NULL) 
	{
		LOG_ERROR(ELOG_KEY, "[system] AgentMaster instance failed");
		exit(0);
	}
	//atexit(ServerExit);

	pMaster->PrepareStart();
	pMaster->SingleStart();

	LOG_SHUTDOWN_ALL;
	return 0;
}
