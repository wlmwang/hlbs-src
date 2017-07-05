
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "AgentMaster.h"
#include "AgentWorker.h"
#include "AgentServer.h"
#include "AgentConfig.h"

int AgentMaster::PrepareRun() {
	return 0;
}

int AgentMaster::NewWorker(uint32_t slot, wWorker** ptr) {
    SAFE_NEW(AgentWorker(mTitle, slot, this), *ptr);
    if (!*ptr) {
        LOG_ERROR(soft::GetLogPath(), "%s : %s", "AgentMaster::NewWorker new() failed", "");
        return -1;
    }
    return 0;
}

int AgentMaster::Reload() {
	AgentConfig* config = mServer->Config<AgentConfig*>();

	int ret = config->ParseBaseConf();
	if (ret == -1) {
		LOG_ERROR(soft::GetLogPath(), "%s : %s", "AgentMaster::Reload ParseBaseConf() failed", "");
		return ret;
	}

	ret = config->ParseRouterConf();
	if (ret == -1) {
		LOG_ERROR(soft::GetLogPath(), "%s : %s", "AgentMaster::Reload ParseRouterConf() failed", "");
		return ret;
	}

	ret = config->ParseQosConf();
	if (ret == -1) {
		LOG_ERROR(soft::GetLogPath(), "%s : %s", "AgentMaster::Reload ParseQosConf() failed", "");
		return ret;
	}
	
	return 0;
}
