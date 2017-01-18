
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "AgentServer.h"
#include "AgentTcpTask.h"
#include "AgentUnixTask.h"
#include "AgentChannelTask.h"

const wStatus& AgentServer::NewTcpTask(wSocket* sock, wTask** ptr) {
	SAFE_NEW(AgentTcpTask(sock, Shard(sock)), *ptr);
	if (*ptr == NULL) {
		return mStatus = wStatus::IOError("AgentServer::NewTcpTask", "AgentTcpTask new failed");
	}
	return mStatus;
}

const wStatus& AgentServer::NewUnixTask(wSocket* sock, wTask** ptr) {
	SAFE_NEW(AgentUnixTask(sock, Shard(sock)), *ptr);
	if (*ptr == NULL) {
		return mStatus = wStatus::IOError("AgentServer::NewUnixTask", "NewUnixTask new failed");
	}
	return mStatus;
}

const wStatus& AgentServer::NewChannelTask(wSocket* sock, wTask** ptr) {
	SAFE_NEW(AgentChannelTask(sock, mMaster, Shard(sock)), *ptr);
    if (*ptr == NULL) {
		return mStatus = wStatus::IOError("AgentServer::AgentChannelTask", "new failed");
    }
    return mStatus;
}
