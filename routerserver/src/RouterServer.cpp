
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "RouterServer.h"
#include "RouterConfig.h"
#include "RouterTcpTask.h"

const wStatus& RouterServer::NewTcpTask(wSocket* sock, wTask** ptr) {
	SAFE_NEW(RouterTcpTask(sock, Shard(sock)), *ptr);
	if (*ptr == NULL) {
		return mStatus = wStatus::IOError("RouterServer::NewTcpTask", "RouterTcpTask new failed");
	}
	return mStatus;
}

const wStatus& RouterServer::Run() {
	return CheckSvr();
}

const wStatus& RouterServer::CheckSvr() {
	RouterConfig* config = Config<RouterConfig*>();

	if (config->GetMtime()) {

		// 增量下发svr.xml更新配置
		struct SvrResSync_t svrSync;
		svrSync.mCode = 0;

		if (config->ParseModifySvr(svrSync.mSvr, &svrSync.mNum).Ok() && svrSync.mNum > 0) {
			Broadcast(reinterpret_cast<char *>(&svrSync), sizeof(svrSync));
		}
	}
	return mStatus.Clear();
}
