
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "RouterChannelTask.h"
#include "RouterConfig.h"
#include "RouterServer.h"
#include "SvrCmd.h"
#include "AgntCmd.h"

RouterChannelTask::RouterChannelTask(wSocket *socket, wMaster *master, int32_t type) : wChannelTask(socket, master, type) {
	On(CMD_SVR_RES, SVR_RES_RELOAD, &RouterChannelTask::ReloadSvrRes, this);
	On(CMD_SVR_RES, SVR_RES_SYNC, &RouterChannelTask::SyncSvrRes, this);

	On(CMD_AGNT_RES, AGNT_RES_RELOAD, &RouterChannelTask::ReloadAgntRes, this);
	On(CMD_AGNT_RES, AGNT_RES_SYNC, &RouterChannelTask::SyncAgntRes, this);
}

int RouterChannelTask::ReloadSvrRes(struct Request_t *request) {
	struct SvrResReload_t* cmd = reinterpret_cast<struct SvrResReload_t*>(request->mBuf);
	RouterConfig* config = Config<RouterConfig*>();
	if (cmd->mCode >= 0 && cmd->mNum > 0) {
		if (cmd->mCode == 0) {	// 清除原始svr
			config->Qos()->CleanNode();
		}
		for (int32_t i = 0; i < cmd->mNum; i++) {
			config->Qos()->SaveNode(cmd->mSvr[i]);
		}
		Server<RouterServer*>()->Broadcast(reinterpret_cast<char*>(cmd), sizeof(struct SvrResReload_t));
	}
	return 0;
}

int RouterChannelTask::SyncSvrRes(struct Request_t *request) {
	struct SvrResSync_t* cmd = reinterpret_cast<struct SvrResSync_t*>(request->mBuf);
	RouterConfig* config = Config<RouterConfig*>();
	if (cmd->mCode >= 0 && cmd->mNum > 0) {
		for (int32_t i = 0; i < cmd->mNum; i++) {
			config->Qos()->SaveNode(cmd->mSvr[i]);
		}
		Server<RouterServer*>()->Broadcast(reinterpret_cast<char*>(cmd), sizeof(struct SvrResSync_t));
	}
	return 0;
}

int RouterChannelTask::ReloadAgntRes(struct Request_t *request) {
	struct AgntResReload_t* cmd = reinterpret_cast<struct AgntResReload_t*>(request->mBuf);
	RouterConfig* config = Config<RouterConfig*>();
	if (cmd->mCode >= 0 && cmd->mNum > 0) {
		if (cmd->mCode == 0) {	// 清除原始agnt
			config->CleanAgnt();
			config->ParseAgntConf();
		}
		for (int32_t i = 0; i < cmd->mNum; i++) {
			config->SaveAgnt(cmd->mAgnt[i]);
		}
		Server<RouterServer*>()->Broadcast(reinterpret_cast<char*>(cmd), sizeof(struct AgntResReload_t));
	}
	return 0;
}

int RouterChannelTask::SyncAgntRes(struct Request_t *request) {
	struct AgntResSync_t* cmd = reinterpret_cast<struct AgntResSync_t*>(request->mBuf);
	RouterConfig* config = Config<RouterConfig*>();
	if (cmd->mCode >= 0 && cmd->mNum > 0) {
		for (int32_t i = 0; i < cmd->mNum; i++) {
			config->SaveAgnt(cmd->mAgnt[i]);
		}
		Server<RouterServer*>()->Broadcast(reinterpret_cast<char*>(cmd), sizeof(struct AgntResSync_t));
	}
	return 0;
}
