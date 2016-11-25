
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "RouterMaster.h"
#include "RouterServer.h"
#include "RouterConfig.h"

const wStatus& RouterMaster::PrepareRun() {
	mWorkerNum = 1;
	return mStatus.Clear();
}

const wStatus& RouterMaster::Reload() {
	RouterConfig* config = mServer->Config<RouterConfig*>();

	mStatus = config->ParseBaseConf();
	if (!mStatus.Ok()) {
		return mStatus;
	}
	mStatus = config->ParseSvrConf();
	if (!mStatus.Ok()) {
		return mStatus;
	}
	mStatus = config->ParseQosConf();
	if (!mStatus.Ok()) {
		return mStatus;
	}
	return mStatus.Clear();
}
