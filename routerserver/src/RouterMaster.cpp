
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "Common.h"
#include "RouterMaster.h"
#include "RouterWorker.h"

const wStatus& RouterMaster::PrepareRun() {
	mWorkerNum = 1;
	return mStatus.Clear();
}

const wStatus& RouterMaster::Reload() {
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
