
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "AgentChannelTask.h"
#include "AgentConfig.h"
#include "SvrCmd.h"

AgentChannelTask::AgentChannelTask(wSocket *socket, wMaster *master, int32_t type = 0) : wChannelTask(socket, master, type) {
	On(CMD_SVR_RES, SVR_RES_INIT, &AgentChannelTask::InitSvrRes, this);
	On(CMD_SVR_RES, SVR_RES_RELOAD, &AgentChannelTask::ReloadSvrRes, this);
	On(CMD_SVR_RES, SVR_RES_SYNC, &AgentChannelTask::SyncSvrRes, this);
}

int AgentChannelTask::InitSvrRes(struct Request_t *request) {
	struct SvrResInit_t* cmd = reinterpret_cast<struct SvrResInit_t*>(request->mBuf);

	AgentConfig* config = Server()->Config<AgentConfig*>();
	if (cmd->mCode == 0 && cmd->mNum > 0) {
		for (int32_t i = 0; i < cmd->mNum; i++) {
			config->Qos()->SaveNode(cmd->mSvr[i]);
		}
	}
	return 0;
}

int AgentChannelTask::ReloadSvrRes(struct Request_t *request) {
	struct SvrResReload_t* cmd = reinterpret_cast<struct SvrResReload_t*>(request->mBuf);

	AgentConfig* config = Server()->Config<AgentConfig*>();
	if (cmd->mCode == 0 && cmd->mNum > 0) {
		// 清除原始svr
		config->Qos()->CleanNode();
		for (int32_t i = 0; i < cmd->mNum; i++) {
			config->Qos()->SaveNode(cmd->mSvr[i]);
		}
	}
	return 0;
}

int AgentChannelTask::SyncSvrRes(struct Request_t *request) {
	struct SvrResSync_t* cmd = reinterpret_cast<struct SvrResSync_t*>(request->mBuf);

	AgentConfig* config = Server()->Config<AgentConfig*>();
	if (cmd->mCode == 0 && cmd->mNum > 0) {
		for (int32_t i = 0; i < cmd->mNum; i++) {
			config->Qos()->ModifyNode(cmd->mSvr[i]);
		}
	}
	return 0;
}
