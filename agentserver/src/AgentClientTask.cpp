
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "AgentClientTask.h"
#include "AgentConfig.h"
#include "SvrCmd.h"
#include "AgntCmd.h"

AgentClientTask::AgentClientTask(wSocket *socket, int32_t type) : wTcpTask(socket, type) {
	On(CMD_SVR_RES, SVR_RES_INIT, &AgentClientTask::InitSvrRes, this);
	On(CMD_SVR_RES, SVR_RES_RELOAD, &AgentClientTask::ReloadSvrRes, this);
	On(CMD_SVR_RES, SVR_RES_SYNC, &AgentClientTask::SyncSvrRes, this);

	On(CMD_AGNT_RES, AGNT_RES_INIT, &AgentClientTask::InitAgntRes, this);
	On(CMD_AGNT_RES, AGNT_RES_RELOAD, &AgentClientTask::ReloadAgntRes, this);
	On(CMD_AGNT_RES, AGNT_RES_SYNC, &AgentClientTask::SyncAgntRes, this);
}

const wStatus& AgentClientTask::Connect() {
	// 发送初始化svr配置请求
	struct SvrReqInit_t vRRt0;
	AsyncSend(reinterpret_cast<char*>(&vRRt0), sizeof(vRRt0));

	/*
	// 上报agent本身信息
	AgentConfig* config = Config<AgentConfig*>();
	std::vector<unsigned int> ips;
	if (misc::GetIpList(ips) == 0) {
		std::sort(ips.begin(), ips.end());
		AgntResSync_t vRRt1;
		for (size_t i = 0; i < ips.size(); i++) {
			if (ips[i] != misc::Text2IP("127.0.0.1")) {
				memcpy(vRRt1.mAgnt[vRRt1.mNum].mHost, misc::IP2Text(ips[i]), kMaxHost);
				vRRt1.mAgnt[vRRt1.mNum].mPort = Socket()->Port();
				vRRt1.mAgnt[vRRt1.mNum].mIdc = config->Qos()->Idc();
				vRRt1.mAgnt[vRRt1.mNum].mStatus = kAgntUreg;
				vRRt1.mNum++;
				break;	// 只上报一个IP
			}
		}
		AsyncSend(reinterpret_cast<char*>(&vRRt1), sizeof(vRRt1));
	}
	*/

	// 发送初始化agent配置请求
	struct AgntReqInit_t vRRt2;
	AsyncSend(reinterpret_cast<char*>(&vRRt2), sizeof(vRRt2));
	return mStatus;
}

const wStatus& AgentClientTask::ReConnect() {
	// 发送重载svr配置请求
	struct SvrReqReload_t vRRt0;
	AsyncSend(reinterpret_cast<char*>(&vRRt0), sizeof(vRRt0));

	// 发送重载agent配置请求
	struct AgntReqReload_t vRRt1;
	AsyncSend(reinterpret_cast<char*>(&vRRt1), sizeof(vRRt1));
	return mStatus;
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

// router发来sync相响应（增量同步）
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

int AgentClientTask::InitAgntRes(struct Request_t *request) {
	struct AgntResInit_t* cmd = reinterpret_cast<struct AgntResInit_t*>(request->mBuf);
	AgentConfig* config = Config<AgentConfig*>();
	if (cmd->mCode >= 0 && cmd->mNum > 0) {
		std::vector<uint32_t> ips;
		if (misc::GetIpList(ips) == 0) {
			for (int32_t i = 0; i < cmd->mNum; i++) {
				uint32_t ip = inet_addr(cmd->mAgnt[i].mHost);
				if (cmd->mAgnt[i].mIdc != 0 && std::find(ips.begin(), ips.end(), ip) != ips.end()) {
					config->Qos()->Idc() = cmd->mAgnt[i].mIdc;
					break;
				}
			}
		}
	}
	return 0;
}

int AgentClientTask::ReloadAgntRes(struct Request_t *request) {
	struct AgntResReload_t* cmd = reinterpret_cast<struct AgntResReload_t*>(request->mBuf);
	AgentConfig* config = Config<AgentConfig*>();
	if (cmd->mCode >= 0 && cmd->mNum > 0) {
		std::vector<uint32_t> ips;
		if (misc::GetIpList(ips) == 0) {
			for (int32_t i = 0; i < cmd->mNum; i++) {
				uint32_t ip = inet_addr(cmd->mAgnt[i].mHost);
				if (cmd->mAgnt[i].mIdc != 0 && std::find(ips.begin(), ips.end(), ip) != ips.end()) {
					config->Qos()->Idc() = cmd->mAgnt[i].mIdc;
					break;
				}
			}
		}
	}
	return 0;
}

int AgentClientTask::SyncAgntRes(struct Request_t *request) {
	struct AgntResSync_t* cmd = reinterpret_cast<struct AgntResSync_t*>(request->mBuf);
	AgentConfig* config = Config<AgentConfig*>();
	if (cmd->mCode >= 0 && cmd->mNum > 0) {
		std::vector<uint32_t> ips;
		if (misc::GetIpList(ips) == 0) {
			for (int32_t i = 0; i < cmd->mNum; i++) {
				uint32_t ip = inet_addr(cmd->mAgnt[i].mHost);
				if (cmd->mAgnt[i].mIdc != 0 && std::find(ips.begin(), ips.end(), ip) != ips.end()) {
					config->Qos()->Idc() = cmd->mAgnt[i].mIdc;
					break;
				}
			}
		}
	}
	return 0;
}
