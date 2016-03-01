
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include "wType.h"
#include "wLog.h"
#include "BaseCommand.h"
#include "RouterConfig.h"
#include "RouterMaster.h"

int main(int argc, const char *argv[])
{
	//config
	RouterConfig *pConfig = RouterConfig::Instance();
	if(pConfig == NULL) 
	{
		cout << "Get RouterConfig instance failed" << endl;
		exit(1);
	}
	if (pConfig->GetOption(argc, argv) < 0)
	{
		cout << "Get Option failed" << endl;
		exit(1);
	}
	pConfig->GetBaseConf();
	pConfig->GetSvrConf();

	//daemon
	if (pConfig->mDaemon == 1)
	{
		if (InitDaemon(LOCK_PATH) < 0)
		{
			LOG_ERROR(ELOG_KEY, "[startup] Create daemon failed!");
			exit(1);
		}
	}
	
	//master
	RouterMaster *pMaster = RouterMaster::Instance();

	if (pMaster->InitMaster() == -1)
	{
		exit(1);
	}
	if (pMaster->CreatePidFile() == -1)
	{
		exit(1);
	}
	
	pMaster->MasterStart();

	LOG_SHUTDOWN_ALL;

	return 0;
}
