
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "AgentClientTask.h"
#include "AgentConfig.h"
#include "SvrCmd.h"
#include "AgntCmd.h"
#include "Misc.h"

AgentClientTask::AgentClientTask(wSocket *socket, int32_t type) : wTcpTask(socket, type) {
	On(CMD_SVR_REQ, SVR_REQ_INIT, &AgentClientTask::InitSvrReq, this);

	On(CMD_SVR_RES, SVR_RES_INIT, &AgentClientTask::InitSvrRes, this);
	On(CMD_SVR_RES, SVR_RES_RELOAD, &AgentClientTask::ReloadSvrRes, this);
	On(CMD_SVR_RES, SVR_RES_SYNC, &AgentClientTask::SyncSvrRes, this);

	On(CMD_AGNT_RES, AGNT_RES_INIT, &AgentClientTask::InitAgntRes, this);
	On(CMD_AGNT_RES, AGNT_RES_RELOAD, &AgentClientTask::ReloadAgntRes, this);
	On(CMD_AGNT_RES, AGNT_RES_SYNC, &AgentClientTask::SyncAgntRes, this);
}

int AgentClientTask::Connect() {
	// v3.0.8起 需手动注册agent主机名
	char hostname[kMaxName];
	if (gethostname(hostname, kMaxName - 1) == 0) {
		std::string ip = FilterLocalIp("0.0.0.0");	// 本机地址
		uint16_t port = 10005;

		struct AgntResSync_t vRRt;
		vRRt.mNum = 1;
		vRRt.mAgnt[0].mStatus = kAgntOk;		
		vRRt.mAgnt[0].mPort = port;
		memcpy(vRRt.mAgnt[0].mHost, ip.c_str(), kMaxHost);
		memcpy(vRRt.mAgnt[0].mName, hostname, kMaxName);
		AsyncSend(reinterpret_cast<char*>(&vRRt), sizeof(vRRt));
	}

	// 初始化SVR记录
	struct SvrReqInit_t vRRt;
	return AsyncSend(reinterpret_cast<char*>(&vRRt), sizeof(vRRt));
}

int AgentClientTask::ReConnect() {
	return Connect();
}

int AgentClientTask::InitSvrReq(struct Request_t *request) {
	struct SvrReqInit_t vRRt;
	return AsyncSend(reinterpret_cast<char*>(&vRRt), sizeof(vRRt));
}

// router发来init相响应
int AgentClientTask::InitSvrRes(struct Request_t *request) {
	struct SvrResInit_t* cmd = reinterpret_cast<struct SvrResInit_t*>(request->mBuf);
	AgentConfig* config = Config<AgentConfig*>();
	if (cmd->mCode >= 0 && cmd->mNum > 0) {
		if (cmd->mCode == 0) {	// 清除原始svr
			config->Qos()->CleanNode();
		}

		for (int32_t i = 0; i < cmd->mNum; i++) {
			config->Qos()->SaveNode(cmd->mSvr[i]);
		}
	}
	return 0;
}

// router发来reload相响应
int AgentClientTask::ReloadSvrRes(struct Request_t *request) {
	struct SvrResReload_t* cmd = reinterpret_cast<struct SvrResReload_t*>(request->mBuf);
	AgentConfig* config = Config<AgentConfig*>();
	if (cmd->mCode >= 0 && cmd->mNum > 0) {
		if (cmd->mCode == 0) {	// 清除原始svr
			config->Qos()->CleanNode();
		}
		
		for (int32_t i = 0; i < cmd->mNum; i++) {
			config->Qos()->SaveNode(cmd->mSvr[i]);
		}
	}
	return 0;
}

// router发来sync相响应，增量同步
int AgentClientTask::SyncSvrRes(struct Request_t *request) {
	struct SvrResSync_t* cmd = reinterpret_cast<struct SvrResSync_t*>(request->mBuf);
	AgentConfig* config = Config<AgentConfig*>();
	if (cmd->mCode >= 0 && cmd->mNum > 0) {
		for (int32_t i = 0; i < cmd->mNum; i++) {
			config->Qos()->SaveNode(cmd->mSvr[i]);
		}
	}
	return 0;
}

// ignore
// v3.0.8 agent自动注册
int AgentClientTask::SyncAgntRes(struct Request_t *request) {
	struct AgntResSync_t* cmd = reinterpret_cast<struct AgntResSync_t*>(request->mBuf);
	AgentConfig* config = Config<AgentConfig*>();

	if (cmd->mCode >= 0 && cmd->mNum > 0) {
		std::vector<uint32_t> ips;
		if (misc::GetIpList(ips) == 0) {
			for (int32_t i = 0; i < cmd->mNum; i++) {
				uint32_t ip = misc::Text2IP(cmd->mAgnt[i].mHost);
				if (std::find(ips.begin(), ips.end(), ip) != ips.end()) {
					config->Qos()->Idc() = cmd->mAgnt[i].mIdc;
					
					if (cmd->mAgnt[i].mWeight == 0 || cmd->mAgnt[i].mStatus == kAgntUreg) {	// 删除或取消注册
						config->Qos()->CleanNode();	// 清除原始svr
					} else if (cmd->mAgnt[i].mStatus == kAgntOk) {	// 发送初始化svr配置请求
						struct SvrReqInit_t vRRt0;
						AsyncSend(reinterpret_cast<char*>(&vRRt0), sizeof(vRRt0));
					}
					break;
				}
			}
		}
	}
	return 0;
}

// ignore
// v3.0.8 agent自动注册
int AgentClientTask::InitAgntRes(struct Request_t *request) {
	struct AgntResInit_t* cmd = reinterpret_cast<struct AgntResInit_t*>(request->mBuf);
	AgentConfig* config = Config<AgentConfig*>();

	if (cmd->mCode >= 0 && cmd->mNum > 0) {
		std::vector<uint32_t> ips;
		if (misc::GetIpList(ips) == 0) {
			for (int32_t i = 0; i < cmd->mNum; i++) {
				uint32_t ip = misc::Text2IP(cmd->mAgnt[i].mHost);
				if (std::find(ips.begin(), ips.end(), ip) != ips.end()) {
					config->Qos()->Idc() = cmd->mAgnt[i].mIdc;

					if (cmd->mAgnt[i].mWeight == 0 || cmd->mAgnt[i].mStatus == kAgntUreg) {	// 删除或取消注册
						config->Qos()->CleanNode();	
					} else if (cmd->mAgnt[i].mStatus == kAgntOk) {	// 发送初始化svr配置请求
						struct SvrReqInit_t vRRt0;
						AsyncSend(reinterpret_cast<char*>(&vRRt0), sizeof(vRRt0));
					}
					break;
				}
			}
		}
	}
	return 0;
}

// ignore
// v3.0.8 agent自动注册
int AgentClientTask::ReloadAgntRes(struct Request_t *request) {
	struct AgntResReload_t* cmd = reinterpret_cast<struct AgntResReload_t*>(request->mBuf);
	AgentConfig* config = Config<AgentConfig*>();

	if (cmd->mCode >= 0 && cmd->mNum > 0) {
		std::vector<uint32_t> ips;
		if (misc::GetIpList(ips) == 0) {
			for (int32_t i = 0; i < cmd->mNum; i++) {
				uint32_t ip = misc::Text2IP(cmd->mAgnt[i].mHost);
				if (std::find(ips.begin(), ips.end(), ip) != ips.end()) {
					config->Qos()->Idc() = cmd->mAgnt[i].mIdc;

					if (cmd->mAgnt[i].mWeight == 0 || cmd->mAgnt[i].mStatus == kAgntUreg) {	// 删除或取消注册
						config->Qos()->CleanNode();	// 清除原始svr
					} else if (cmd->mAgnt[i].mStatus == kAgntOk) {	// 发送初始化svr配置请求
						struct SvrReqInit_t vRRt0;
						AsyncSend(reinterpret_cast<char*>(&vRRt0), sizeof(vRRt0));
					}
					break;
				}
			}
		}
	}
	return 0;
}
