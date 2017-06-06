
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "RouterMaster.h"
#include "RouterServer.h"
#include "RouterConfig.h"

const wStatus& RouterMaster::PrepareRun() {
	return mStatus;
}

const wStatus& RouterMaster::Reload() {
	RouterConfig* config = mServer->Config<RouterConfig*>();
	if (config->ParseBaseConf() == -1) {
		mStatus = wStatus::IOError("RouterMaster::Reload ParseBaseConf() failed", "");
	}
	return mStatus;
}
