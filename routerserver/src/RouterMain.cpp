
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "wCore.h"
#include "wLog.h"
#include "RouterConfig.h"
#include "RouterMaster.h"

int main(int argc, const char *argv[])
{
	//config
	RouterConfig *pConfig = RouterConfig::Instance();
	if(pConfig == NULL) 
	{
		cout << "[startup] Get RouterConfig instance failed" << endl;
		exit(2);
	}
	if (pConfig->GetOption(argc, argv) < 0)
	{
		cout << "[startup] Get Option failed" << endl;
		exit(2);
	}
	pConfig->GetBaseConf();
	pConfig->GetSvrConf();
	pConfig->GetQosConf();
	
	//daemon
	if (pConfig->mDaemon == 1)
	{
		if (InitDaemon(LOCK_PATH) < 0)
		{
			LOG_ERROR(ELOG_KEY, "[startup] Create daemon failed!");
			exit(2);
		}
	}
	
	//master
	RouterMaster *pMaster = RouterMaster::Instance();
	if(pMaster == NULL) 
	{
		LOG_ERROR(ELOG_KEY, "[startup] Get RouterMaster instance failed");
		exit(2);
	}
	pMaster->PrepareStart();
	pMaster->MasterStart();

	LOG_SHUTDOWN_ALL;
	return 0;
}
