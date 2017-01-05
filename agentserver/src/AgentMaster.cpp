
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "AgentMaster.h"
#include "AgentWorker.h"
#include "AgentServer.h"
#include "AgentConfig.h"

const wStatus& AgentMaster::PrepareRun() {
	mWorkerNum = mNcpu;
	return mStatus.Clear();
}

const wStatus& AgentMaster::NewWorker(uint32_t slot, wWorker** ptr) {
    SAFE_NEW(AgentWorker(mTitle, slot, this), *ptr);
    if (*ptr == NULL) {
		return mStatus = wStatus::IOError("AgentMaster::NewWorker", "new failed");
    }
    return mStatus.Clear();
}

const wStatus& AgentMaster::Reload() {
	AgentConfig* config = mServer->Config<AgentConfig*>();

	if (!(mStatus = config->ParseBaseConf()).Ok()) {
		return mStatus;
	} else if (!(mStatus = config->ParseRouterConf()).Ok()) {
		return mStatus;
	} else if (!(mStatus = config->ParseQosConf()).Ok()) {
		return mStatus;
	}
	return mStatus.Clear();
}
