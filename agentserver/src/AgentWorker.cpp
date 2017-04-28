
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "AgentWorker.h"
#include "AgentClient.h"
#include "AgentMaster.h"
#include "AgentConfig.h"

AgentWorker::AgentWorker(std::string title, uint32_t slot, wMaster* master) : wWorker(title, slot, master) {
	SAFE_NEW(AgentClient(Master()->Server()->Config(), Master()->Server()), mAgentClient);
}

AgentWorker::~AgentWorker() {
	SAFE_DELETE(mAgentClient);
}

const wStatus& AgentWorker::PrepareRun() {
	if (!(mStatus = mAgentClient->PrepareStart()).Ok()) {
		return mStatus;
	} else if (!(mStatus = mAgentClient->StartThread()).Ok()) {
		return mStatus;
	}

	if (!(mStatus = Master()->Server()->Config<AgentConfig*>()->Qos()->StartDetectThread()).Ok()) {
		return mStatus;
	}
	return mStatus;
}
