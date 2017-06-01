
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "RouterServer.h"
#include "RouterMaster.h"
#include "RouterConfig.h"
#include "RouterTcpTask.h"
#include "RouterHttpTask.h"
#include "RouterChannelTask.h"

const wStatus& RouterServer::NewTcpTask(wSocket* sock, wTask** ptr) {
	SAFE_NEW(RouterTcpTask(sock, Shard(sock)), *ptr);
	if (*ptr == NULL) {
		return mStatus = wStatus::IOError("RouterServer::NewTcpTask", "RouterTcpTask new failed");
	}
	return mStatus;
}

const wStatus& RouterServer::NewHttpTask(wSocket* sock, wTask** ptr) {
	SAFE_NEW(RouterHttpTask(sock, Shard(sock)), *ptr);
	if (*ptr == NULL) {
		return mStatus = wStatus::IOError("RouterServer::NewHttpTask", "RouterHttpTask new failed");
	}
	return mStatus;
}

const wStatus& RouterServer::NewChannelTask(wSocket* sock, wTask** ptr) {
	SAFE_NEW(RouterChannelTask(sock, Master<RouterMaster*>(), Shard(sock)), *ptr);
    if (*ptr == NULL) {
		return mStatus = wStatus::IOError("RouterServer::RouterChannelTask", "new failed");
    }
    return mStatus;
}

const wStatus& RouterServer::PrepareRun() {
	// 开启control监听
    RouterConfig* config = Config<RouterConfig*>();

    std::string host;
    int16_t port = 0;
    if (!config->GetConf("control_host", &host) || !config->GetConf("control_port", &port)) {
    	return mStatus = wStatus::Corruption("RouterServer::PrepareRun failed", "host or port is illegal");
    }

    std::string protocol;
    if (!config->GetConf("control_protocol", &protocol)) {
    	mStatus = AddListener(host, port, "TCP");
    } else {
    	mStatus = AddListener(host, port, protocol);
    }
    return mStatus;
}

const wStatus& RouterServer::Run() {
	if (Config<RouterConfig*>()->IsModifySvr()) {
		Config<RouterConfig*>()->Qos()->CleanNode();
		Config<RouterConfig*>()->ParseSvrConf();
	}
	return mStatus;
}
