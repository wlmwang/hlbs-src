
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
	} else if (!config->CleanAgnt() || config->ParseAgntConf() == -1) {
		mStatus = wStatus::IOError("RouterMaster::Reload ParseAgntConf() failed", "");
	} else if (!config->Qos()->CleanNode().Ok() || config->ParseSvrConf() == -1) {
		mStatus = wStatus::IOError("RouterMaster::Reload ParseSvrConf() failed", "");
	}
	return mStatus;
}
