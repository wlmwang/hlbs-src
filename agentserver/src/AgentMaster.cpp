
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "Common.h"
#include "AgentMaster.h"

const wStatus& AgentMaster::PrepareRun() {
	return mStatus.Clear();
}

const wStatus& AgentMaster::Reload() {
	mStatus = config->ParseBaseConf();
	if (!mStatus.Ok()) {
		return mStatus;
	}
	mStatus = config->ParseRouterConf();
	if (!mStatus.Ok()) {
		return mStatus;
	}
	mStatus = config->ParseQosConf();
	if (!mStatus.Ok()) {
		return mStatus;
	}
	return mStatus.Clear();
}
