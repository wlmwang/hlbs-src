
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "RouterMaster.h"
#include "RouterServer.h"
#include "RouterConfig.h"

int RouterMaster::PrepareRun() {
	return 0;
}

int RouterMaster::Reload() {
	RouterConfig* config = mServer->Config<RouterConfig*>();
	
	int ret = 0;
	ret = config->ParseBaseConf();
	if (ret == -1) {
		HNET_ERROR(soft::GetLogPath(), "%s : %s", "RouterMaster::Reload ParseBaseConf() failed", "");
		return -1;
	}

	config->CleanAgnt();
	if (config->ParseAgntConf() == -1) {
		HNET_ERROR(soft::GetLogPath(), "%s : %s", "RouterMaster::Reload ParseAgntConf() failed", "");
		return ret;
	}

	if (config->Qos()->CleanNode() == -1 || config->ParseSvrConf() == -1) {
		HNET_ERROR(soft::GetLogPath(), "%s : %s", "RouterMaster::Reload ParseSvrConf() failed", "");
		return ret;
	}
	return ret;
}
