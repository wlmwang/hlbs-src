
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "AgentMaster.h"
#include "AgentWorker.h"
#include "AgentServer.h"
#include "AgentConfig.h"

const wStatus& AgentMaster::PrepareRun() {
	return mStatus;
}

const wStatus& AgentMaster::NewWorker(uint32_t slot, wWorker** ptr) {
    SAFE_NEW(AgentWorker(mTitle, slot, this), *ptr);
    if (*ptr == NULL) {
		return mStatus = wStatus::IOError("AgentMaster::NewWorker", "new failed");
    }
    return mStatus;
}

const wStatus& AgentMaster::Reload() {
	AgentConfig* config = mServer->Config<AgentConfig*>();
	if (config->ParseBaseConf() == -1) {
		return mStatus = wStatus::IOError("AgentMaster::Reload ParseBaseConf() failed", "");
	} else if (config->ParseRouterConf() == -1) {
		return mStatus = wStatus::IOError("AgentMaster::Reload ParseRouterConf() failed", "");
	} else if (config->ParseQosConf() == -1) {
		return mStatus = wStatus::IOError("AgentMaster::Reload ParseQosConf() failed", "");
	}
	return mStatus;
}
