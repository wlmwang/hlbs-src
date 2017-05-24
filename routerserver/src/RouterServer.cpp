
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "RouterServer.h"
#include "RouterConfig.h"
#include "RouterTcpTask.h"
#include "RouterHttpTask.h"

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
	return mStatus;
}
