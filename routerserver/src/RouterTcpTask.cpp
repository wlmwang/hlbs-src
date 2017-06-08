
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "wLogger.h"
#include "RouterTcpTask.h"
#include "RouterConfig.h"
#include "SvrCmd.h"
#include "AgntCmd.h"

RouterTcpTask::RouterTcpTask(wSocket *socket, int32_t type) : wTcpTask(socket, type) {
	On(CMD_SVR_REQ, SVR_REQ_INIT, &RouterTcpTask::InitSvrReq, this);
	On(CMD_SVR_REQ, SVR_REQ_RELOAD, &RouterTcpTask::ReloadSvrReq, this);

	On(CMD_AGNT_REQ, AGNT_REQ_INIT, &RouterTcpTask::InitAgntReq, this);
	On(CMD_AGNT_REQ, AGNT_REQ_RELOAD, &RouterTcpTask::ReloadAgntReq, this);
	On(CMD_AGNT_RES, AGNT_RES_SYNC, &RouterTcpTask::SyncAgntRes, this);
}

const wStatus& RouterTcpTask::Connect() {
	// 以agent上报为准
	RouterConfig* config = Config<RouterConfig*>();
	std::string ip = Socket()->Host();
	uint16_t port = Socket()->Port();
	struct Agnt_t agnt, old;

	// 过滤router与agent部署在同一台机器的特殊情况
	if (misc::Text2IP(ip.c_str()) == misc::Text2IP("127.0.0.1")) {
		std::vector<unsigned int> ips;
		if (misc::GetIpList(ips) == 0) {
			std::sort(ips.begin(), ips.end());
			for (size_t i = 0; i < ips.size(); i++) {
				if (ips[i] != misc::Text2IP("127.0.0.1")) {
					ip = misc::IP2Text(ips[i]);
					break;
				}
			}
		}
	}

	memcpy(agnt.mHost, ip.c_str(), kMaxHost);
	agnt.mPort = port;
	agnt.mStatus = kAgntUreg;
	agnt.mVersion = misc::GetTimeofday()/1000000;

	// 更新本进程
	if (config->IsExistAgnt(agnt, &old) && old.mConfig == 0) {
		agnt.mConfig = old.mConfig;
		agnt.mStatus = kAgntOk;
	}
	config->SaveAgnt(agnt);

	// 同步更新其他进程
	AgntResSync_t vRRt;
	vRRt.mCode = 0;
	vRRt.mNum = 1;
	vRRt.mAgnt[0] = agnt;
	SyncWorker(reinterpret_cast<char*>(&vRRt), sizeof(vRRt));
	return mStatus;
}

const wStatus& RouterTcpTask::DisConnect() {
	RouterConfig* config = Config<RouterConfig*>();
	std::string ip = Socket()->Host();
	uint16_t port = Socket()->Port();
	struct Agnt_t agnt, old;

	// 过滤router与agent部署在同一台机器的特殊情况
	if (misc::Text2IP(ip.c_str()) == misc::Text2IP("127.0.0.1")) {
		std::vector<unsigned int> ips;
		if (misc::GetIpList(ips) == 0) {
			std::sort(ips.begin(), ips.end());
			for (size_t i = 0; i < ips.size(); i++) {
				if (ips[i] != misc::Text2IP("127.0.0.1")) {
					ip = misc::IP2Text(ips[i]);
					break;
				}
			}
		}
	}

	memcpy(agnt.mHost, ip.c_str(), kMaxHost);
	agnt.mPort = port;
	agnt.mStatus = kAgntDisc;
	agnt.mVersion = misc::GetTimeofday()/1000000;

	// 更新本进程
	if (config->IsExistAgnt(agnt, &old)) {
		agnt.mConfig = old.mConfig;
		if (agnt.mVersion < old.mVersion + 1) {	// 1秒之内认定agent抖动（重启也是抖动）
			return mStatus;
		}
	}
	config->SaveAgnt(agnt);

	// 同步更新其他进程
	AgntResSync_t vRRt;
	vRRt.mCode = 0;
	vRRt.mNum = 1;
	vRRt.mAgnt[0] = agnt;
	SyncWorker(reinterpret_cast<char*>(&vRRt), sizeof(vRRt));
	return mStatus;
}

// 向单个agent发送init回应
int RouterTcpTask::InitSvrReq(struct Request_t *request) {
	RouterConfig* config = Config<RouterConfig*>();
	// 获取所有节点
	int32_t start = 0;
	struct SvrResInit_t vRRt;
	do {
		if (!config->Qos()->GetNodeAll(vRRt.mSvr, &vRRt.mNum, start, kMaxNum).Ok() || vRRt.mNum <= 0) {
			break;
		}
		AsyncSend(reinterpret_cast<char*>(&vRRt), sizeof(vRRt));
		vRRt.mCode++;
		start += vRRt.mNum;
	} while (vRRt.mNum >= kMaxNum);
	return 0;
}

// 向单个agent发送reload响应
int RouterTcpTask::ReloadSvrReq(struct Request_t *request) {
	RouterConfig* config = Config<RouterConfig*>();
	// 重新加载配置文件
	config->Qos()->CleanNode();
	config->ParseSvrConf();
	
	int32_t start = 0;
	struct SvrResReload_t vRRt;
	do {
		if (!config->Qos()->GetNodeAll(vRRt.mSvr, &vRRt.mNum, start, kMaxNum).Ok() || vRRt.mNum <= 0) {
			break;
		}
		AsyncSend(reinterpret_cast<char*>(&vRRt), sizeof(vRRt));
		vRRt.mCode++;
		start += vRRt.mNum;
	} while (vRRt.mNum >= kMaxNum);
	return 0;
}

int RouterTcpTask::InitAgntReq(struct Request_t *request) {
	RouterConfig* config = Config<RouterConfig*>();
	int32_t start = 0;
	struct AgntResInit_t vRRt;
	do {
		vRRt.mNum = config->GetAgntAll(vRRt.mAgnt, start, kMaxNum);
		if (vRRt.mNum <= 0) {
			break;
		}
		AsyncSend(reinterpret_cast<char*>(&vRRt), sizeof(vRRt));
		vRRt.mCode++;
		start += vRRt.mNum;
	} while (vRRt.mNum >= kMaxNum);
	return 0;
}

int RouterTcpTask::ReloadAgntReq(struct Request_t *request) {
	RouterConfig* config = Config<RouterConfig*>();
	// 重新加载配置文件
	config->CleanAgnt();
	config->ParseAgntConf();
	
	int32_t start = 0;
	struct AgntResReload_t vRRt;
	do {
		vRRt.mNum = config->GetAgntAll(vRRt.mAgnt, start, kMaxNum);
		if (vRRt.mNum <= 0) {
			break;
		}
		AsyncSend(reinterpret_cast<char*>(&vRRt), sizeof(vRRt));
		vRRt.mCode++;
		start += vRRt.mNum;
	} while (vRRt.mNum >= kMaxNum);	
	return 0;
}

int RouterTcpTask::SyncAgntRes(struct Request_t *request) {
	struct AgntResSync_t* cmd = reinterpret_cast<struct AgntResSync_t*>(request->mBuf);
	RouterConfig* config = Config<RouterConfig*>();
	if (cmd->mCode >= 0 && cmd->mNum > 0) {
		for (int32_t i = 0; i < cmd->mNum; i++) {
			struct Agnt_t old;
			if (cmd->mAgnt[i].mStatus == kAgntUreg && config->IsExistAgnt(cmd->mAgnt[i], &old) && old.mConfig == 0) {
				cmd->mAgnt[i].mStatus = kAgntOk;
				cmd->mAgnt[i].mConfig = old.mConfig;
				cmd->mAgnt[i].mVersion = misc::GetTimeofday()/1000000;
			}
			config->SaveAgnt(cmd->mAgnt[i]);
		}
		SyncWorker(request->mBuf, request->mLen);
	}
	return 0;
}
