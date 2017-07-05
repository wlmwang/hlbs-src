
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

int AgentWorker::PrepareRun() {
	int ret = mAgentClient->PrepareStart();
	if (ret == -1) {
		LOG_ERROR(soft::GetLogPath(), "%s : %s", "AgentWorker::PrepareRun PrepareStart() failed", "");
		return ret;
	}

	ret = mAgentClient->StartThread();
	if (ret == -1) {
		LOG_ERROR(soft::GetLogPath(), "%s : %s", "AgentWorker::PrepareRun StartThread() failed", "");
		return ret;
	}

	ret = Master()->Server()->Config<AgentConfig*>()->Qos()->StartDetectThread();
	if (ret == -1) {
		LOG_ERROR(soft::GetLogPath(), "%s : %s", "AgentWorker::PrepareRun StartDetectThread() failed", "");
		return ret;
	}
	return ret;
}
