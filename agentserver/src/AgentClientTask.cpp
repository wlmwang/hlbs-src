
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "AgentClientTask.h"
#include "AgentConfig.h"
#include "SvrCmd.h"

AgentClientTask::AgentClientTask(wSocket *socket, int32_t type) : wTcpTask(socket, type) {
	On(CMD_SVR_RES, SVR_RES_INIT, &AgentClientTask::InitSvrRes, this);
	On(CMD_SVR_RES, SVR_RES_RELOAD, &AgentClientTask::ReloadSvrRes, this);
	On(CMD_SVR_RES, SVR_RES_SYNC, &AgentClientTask::SyncSvrRes, this);
}

// router发来init相响应
int AgentClientTask::InitSvrRes(struct Request_t *request) {
	struct SvrResInit_t* cmd = reinterpret_cast<struct SvrResInit_t*>(request->mBuf);
	AgentConfig* config = Config<AgentConfig*>();

	if (cmd->mCode == 0 && cmd->mNum > 0) {
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

	if (cmd->mCode == 0 && cmd->mNum > 0) {
		// 清除原始svr
		config->Qos()->CleanNode();
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

	if (cmd->mCode == 0 && cmd->mNum > 0) {
		for (int32_t i = 0; i < cmd->mNum; i++) {
			config->Qos()->ModifyNode(cmd->mSvr[i]);
		}
	}
	return 0;
}
