
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "RouterMaster.h"
#include "RouterServer.h"
#include "RouterConfig.h"

const wStatus& RouterMaster::PrepareRun() {
	mWorkerNum = 1;
	return mStatus;
}

const wStatus& RouterMaster::Reload() {
	RouterConfig* config = mServer->Config<RouterConfig*>();
	
	if (!(mStatus = config->Qos()->CleanNode()).Ok()) {
		return mStatus;
	} else if (!(mStatus = config->ParseBaseConf()).Ok()) {
		return mStatus;
	} else if (!(mStatus = config->ParseSvrConf()).Ok()) {
		return mStatus;
	} else if (!(mStatus = config->ParseQosConf()).Ok()) {
		return mStatus;
	}
	return mStatus;
}
