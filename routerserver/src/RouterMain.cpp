
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "wCore.h"
#include "wLog.h"
#include "RouterConfig.h"
#include "RouterMaster.h"

void ServerExit()
{
	RouterConfig *pConfig = RouterConfig::Instance();
	if (pConfig) 
	{
		SAFE_DELETE(pConfig);
	}
	RouterServer *pServer = RouterServer::Instance();
	if (pServer)
	{
		SAFE_DELETE(pServer);
	}
	RouterMaster *pMaster = RouterMaster::Instance();
	if (pMaster) 
	{
		SAFE_DELETE(pMaster);
	}
}

int main(int argc, const char *argv[])
{
	//config
	RouterConfig *pConfig = RouterConfig::Instance();
	if (pConfig == NULL) 
	{
		cout << "[system] RouterConfig instance failed" << endl;
		exit(2);
	}
	if (pConfig->GetOption(argc, argv) < 0)
	{
		cout << "[system] Command line Option failed" << endl;
		exit(2);
	}
	pConfig->GetBaseConf();
	pConfig->GetSvrConf();
	pConfig->GetQosConf();
	
	//daemon
	if (pConfig->mDaemon == 1)
	{
		if (InitDaemon("../log/hlbs.lock") < 0)
		{
			LOG_ERROR(ELOG_KEY, "[system] Create daemon failed");
			exit(2);
		}
	}
	//master
	RouterMaster *pMaster = RouterMaster::Instance();
	if (pMaster == NULL) 
	{
		LOG_ERROR(ELOG_KEY, "[system] RouterMaster instance failed");
		exit(2);
	}
	atexit(ServerExit);

	pMaster->PrepareStart();
	pMaster->MasterStart();

	LOG_SHUTDOWN_ALL;
	return 0;
}
