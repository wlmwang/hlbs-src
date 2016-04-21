
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
	AgentConfig *pConfig = AgentConfig::Instance();
	if (pConfig) 
	{
		SAFE_DELETE(pConfig);
	}
	AgentServer *pServer = AgentServer::Instance();
	if (pServer)
	{
		SAFE_DELETE(pServer);
	}
	AgentMaster *pMaster = AgentMaster::Instance();
	if (pMaster) 
	{
		SAFE_DELETE(pMaster);
	}
}

int main(int argc, const char *argv[])
{
	//config
	AgentConfig *pConfig = AgentConfig::Instance();
	if(pConfig == NULL) 
	{
		cout << "[system] AgentConfig instance failed" << endl;
		exit(2);
	}
	if (pConfig->GetOption(argc, argv) < 0)
	{
		cout << "[system] Command line Option failed" << endl;
		exit(2);
	}
	pConfig->GetBaseConf();
	pConfig->GetRouterConf();
	pConfig->GetQosConf();
	
	//daemon
	if (pConfig->mDaemon == 1)
	{
		if (InitDaemon("../log/hlbs.lock") < 0)
		{
			LOG_ERROR(ELOG_KEY, "[system] Create daemon failed!");
			exit(2);
		}
	}

	//master
	AgentMaster *pMaster = AgentMaster::Instance();
	if(pMaster == NULL) 
	{
		LOG_ERROR(ELOG_KEY, "[system] AgentMaster instance failed");
		exit(2);
	}
	//atexit(ServerExit);

	pMaster->PrepareStart();
	pMaster->SingleStart();

	LOG_SHUTDOWN_ALL;
	return 0;
}
