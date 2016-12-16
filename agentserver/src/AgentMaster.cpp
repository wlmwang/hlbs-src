
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "AgentMaster.h"
#include "AgentServer.h"
#include "AgentConfig.h"

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
