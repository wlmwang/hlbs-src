
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "AgentClient.h"
#include "AgentServer.h"
#include "AgentConfig.h"
#include "AgentClientTask.h"

const wStatus& AgentClient::NewTcpTask(wSocket* sock, wTask** ptr, int type) {
	SAFE_NEW(AgentClientTask(sock, type), *ptr);
	if (*ptr == NULL) {
		return mStatus = wStatus::IOError("AgentClient::NewTcpTask", "new AgentClientTask failed");
	}
	return mStatus;
}

const wStatus& AgentClient::PrepareRun() {
    wConfig* config = Config<AgentConfig*>();
    if (config == NULL || !config->GetConf("router_host", &mHost) || !config->GetConf("router_port", &mPort)) {
    	return mStatus = wStatus::IOError("AgentClient::PrepareRun failed", "Config() is null or host|port is illegal");
    }

	// 连接RouterSvr服务器(连接失败让其正常启动)
	if (!(mStatus = AddConnect(kType, mHost, mPort)).Ok()) {
		return mStatus.Clear();	
	}
	mInitClient = true;
	return mStatus;
}

const wStatus& AgentClient::Run() {
	if (mInitClient == false) {
		if ((mStatus = AddConnect(kType, mHost, mPort)).Ok()) {
			mInitClient = true;
		}
	}
	return mStatus;
}