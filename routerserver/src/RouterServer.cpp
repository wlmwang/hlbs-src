
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

	if (config->IsModifySvr()) {
		// 增量下发svr.xml更新配置
		int32_t start = 0;
		struct SvrResSync_t vRRt;
		do {
			if (config->ParseModifySvr(vRRt.mSvr, &vRRt.mNum, start, kMaxNum).Ok() && vRRt.mNum > 0) {
				vRRt.mCode = 0;
				Broadcast(reinterpret_cast<char *>(&vRRt), sizeof(vRRt));
			}
			start += vRRt.mNum;
		} while (vRRt.mNum >= kMaxNum);
	}
	return mStatus.Clear();
}
