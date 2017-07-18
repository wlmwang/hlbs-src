
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "AgentClient.h"
#include "AgentServer.h"
#include "AgentConfig.h"
#include "AgentClientTask.h"

int AgentClient::NewTcpTask(wSocket* sock, wTask** ptr, int type) {
	HNET_NEW(AgentClientTask(sock, type), *ptr);
    if (!*ptr) {
        HNET_ERROR(soft::GetLogPath(), "%s : %s", "AgentClient::NewTcpTask new() failed", "");
        return -1;
    }
	return 0;
}

int AgentClient::PrepareRun() {
    wConfig* config = Config<AgentConfig*>();
    if (!config || !config->GetConf("router_host", &mHost) || !config->GetConf("router_port", &mPort)) {
    	HNET_ERROR(soft::GetLogPath(), "%s : %s", "AgentClient::PrepareRun GetConf() failed", "");
    	return -1;
    }

	// 连接RouterSvr服务器(连接失败让其正常启动)
	int ret = AddConnect(kType, mHost, mPort);
	if (ret == 0) {
		mInitClient = true;
	}
	return 0;
}

// 连接RouterSvr服务器
int AgentClient::Run() {
	int ret = 0;
	if (mInitClient == false) {
		ret = AddConnect(kType, mHost, mPort);
		if (ret == 0) {
			mInitClient = true;
		}
	}
	return ret;
}