
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
	//code...
}

int main(int argc, const char *argv[])
{
	//config
	RouterConfig *pConfig = RouterConfig::Instance();
	if (pConfig == NULL) 
	{
		cout << "[system] RouterConfig instance failed" << endl;
		exit(0);
	}
	if (pConfig->GetOption(argc, argv) < 0)
	{
		cout << "[system] Command line Option failed" << endl;
		exit(0);
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
			exit(0);
		}
	}
	//master
	RouterMaster *pMaster = RouterMaster::Instance();
	if (pMaster == NULL) 
	{
		LOG_ERROR(ELOG_KEY, "[system] RouterMaster instance failed");
		exit(0);
	}
	//atexit(ServerExit);

	pMaster->PrepareStart();
	pMaster->MasterStart();

	LOG_SHUTDOWN_ALL;
	return 0;
}
