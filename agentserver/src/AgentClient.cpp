
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "AgentClient.h"
#include "AgentServer.h"
#include "AgentConfig.h"
#include "AgentClientTask.h"
#include "SvrCmd.h"

const wStatus& AgentClient::NewTcpTask(wSocket* sock, wTask** ptr, int type) {
	SAFE_NEW(AgentClientTask(sock, type), *ptr);
	if (*ptr == NULL) {
		return mStatus = wStatus::IOError("AgentClient::NewTcpTask", "new AgentClientTask failed");
	}
	return mStatus;
}

const wStatus& AgentClient::PrepareRun() {
    std::string host;
    int16_t port = 0;

    wConfig* config = Config<AgentConfig*>();
    if (config == NULL || !config->GetConf("router_host", &host) || !config->GetConf("router_port", &port)) {
    	return mStatus = wStatus::IOError("AgentClient::PrepareRun failed", "Config() is null or host|port is illegal");
    }

    // 连接服务器
	if (!AddConnect(kType, host, port).Ok()) {
		return mStatus;
	}
	return InitSvrReq();
}

const wStatus& AgentClient::InitSvrReq() {
	struct SvrReqInit_t svr;
	return Broadcast(reinterpret_cast<char*>(&svr), sizeof(svr), kType);
}

const wStatus& AgentClient::ReloadSvrReq() {
	struct SvrReqReload_t svr;
	return Broadcast(reinterpret_cast<char*>(&svr), sizeof(svr), kType);
}
