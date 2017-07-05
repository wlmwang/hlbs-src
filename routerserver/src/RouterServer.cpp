
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

int RouterServer::NewTcpTask(wSocket* sock, wTask** ptr) {
	SAFE_NEW(RouterTcpTask(sock, Shard(sock)), *ptr);
	if (!*ptr) {
        LOG_ERROR(soft::GetLogPath(), "%s : %s", "RouterServer::NewTcpTask new() failed", "");
        return -1;
	}
	return 0;
}

int RouterServer::NewHttpTask(wSocket* sock, wTask** ptr) {
	SAFE_NEW(RouterHttpTask(sock, Shard(sock)), *ptr);
    if (!*ptr) {
        LOG_ERROR(soft::GetLogPath(), "%s : %s", "RouterServer::NewHttpTask new() failed", "");
        return -1;
    }
	return 0;
}

int RouterServer::NewChannelTask(wSocket* sock, wTask** ptr) {
	SAFE_NEW(RouterChannelTask(sock, Master<RouterMaster*>(), Shard(sock)), *ptr);
    if (!*ptr) {
        LOG_ERROR(soft::GetLogPath(), "%s : %s", "RouterServer::RouterChannelTask new() failed", "");
        return -1;
    }
    return 0;
}

int RouterServer::PrepareRun() {
	// 开启control监听
    RouterConfig* config = Config<RouterConfig*>();

    std::string host;
    int16_t port = 0;
    if (!config->GetConf("control_host", &host) || !config->GetConf("control_port", &port)) {
        LOG_ERROR(soft::GetLogPath(), "%s : %s", "RouterServer::PrepareRun () failed", "host or port invalid");
        return -1;
    }

    int ret = 0;
    std::string protocol;
    if (!config->GetConf("control_protocol", &protocol)) {
    	ret = AddListener(host, port, "TCP");
    } else {
    	ret = AddListener(host, port, protocol);
    }
    return ret;
}

int RouterServer::Run() {
	return 0;
}
