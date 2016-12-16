
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "AgentServer.h"
#include "AgentConfig.h"
#include "AgentTcpTask.h"
#include "AgentClient.h"

AgentServer::AgentServer(wConfig* config) : wServer(config) {
	SAFE_NEW(AgentClient(config), mAgentClient);
}

AgentServer::~AgentServer() {
	SAFE_DELETE(mAgentClient);
}

const wStatus& AgentServer::NewTcpTask(wSocket* sock, wTask** ptr) {
	SAFE_NEW(AgentTcpTask(sock, Shard(sock)), *ptr);
	if (*ptr == NULL) {
		return mStatus = wStatus::IOError("AgentServer::NewTcpTask", "AgentTcpTask new failed");
	}
	return mStatus;
}

/*
virtual const wStatus& AgentServer::NewChannelTask(wSocket* sock, wTask** ptr) {
	SAFE_NEW(AgentChannelTask(sock, mMaster, Shard(sock)), *ptr);
    if (*ptr == NULL) {
		return mStatus = wStatus::IOError("AgentServer::AgentChannelTask", "new failed");
    }
    return mStatus;
}
*/

const wStatus& AgentServer::PrepareRun() {
	if (!(mStatus = mAgentClient->PrepareStart()).Ok()) {
		return mStatus;
	} else if (!(mStatus = mAgentClient->StartThread()).Ok()) {
		return mStatus;
	}
	// 初始化路由
	return mStatus = mAgentClient->InitSvrReq();
}
