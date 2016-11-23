
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "RouterServer.h"
#include "RouterTcpTask.h"

const wStatus& RouterServer::NewTcpTask(wSocket* sock, wTask** ptr) {
	SAFE_NEW(RouterTcpTask(sock, Shard(sock)), *ptr);
	if (*ptr == NULL) {
		return mStatus = wStatus::IOError("RouterServer::NewTcpTask", "RouterTcpTask new failed");
	}
	return mStatus;
}

const wStatus& RouterServer::Run() {
	return CheckModSvr();
}

const wStatus& RouterServer::CheckModSvr() {
	if (mConfig->IsModTime()) {
		struct SvrResSync_t svrSync;
		svrSync.mCode = 0;

		mConfig->ParseModSvr(svrSync.mSvr, &stSvr.mNum);
		if (stSvr.mNum > 0) {
			Broadcast(reinterpret_cast<char *>(&svrSync), sizeof(svrSync));
		}
	}
	return mStatus.Clear();
}
