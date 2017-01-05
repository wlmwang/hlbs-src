
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "AgentChannelTask.h"
#include "AgentConfig.h"
#include "SvrCmd.h"

AgentChannelTask::AgentChannelTask(wSocket *socket, wMaster *master, int32_t type) : wChannelTask(socket, master, type) {
	On(CMD_SVR_RES, SVR_RES_INIT, &AgentChannelTask::InitSvrRes, this);
	On(CMD_SVR_RES, SVR_RES_RELOAD, &AgentChannelTask::ReloadSvrRes, this);
	On(CMD_SVR_RES, SVR_RES_SYNC, &AgentChannelTask::SyncSvrRes, this);
	On(CMD_SVR_RES, SVR_REQ_GXID, &AgentChannelTask::GetSvrByGXid, this);
	On(CMD_SVR_RES, SVR_REQ_REPORT, &AgentChannelTask::ReportSvr, this);
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

int AgentChannelTask::GetSvrByGXid(struct Request_t *request) {
	struct SvrReqGXid_t* cmd = reinterpret_cast<struct SvrReqGXid_t*>(request->mBuf);
	AgentConfig* config = Server()->Config<AgentConfig*>();
	struct SvrOneRes_t vRRt;

	vRRt.mSvr.mGid = cmd->mGid;
	vRRt.mSvr.mXid = cmd->mXid;
	if (config->Qos()->QueryNode(vRRt.mSvr).Ok()) {
		vRRt.mCode = 0;
		vRRt.mNum = 1;
	}
	return 0;
}

int AgentChannelTask::ReportSvr(struct Request_t *request) {
	struct SvrReqReport_t* cmd = reinterpret_cast<struct SvrReqReport_t*>(request->mBuf);
	AgentConfig* config = Server()->Config<AgentConfig*>();

	struct SvrResReport_t vRRt;
	if (config->Qos()->CallerNode(cmd->mCaller).Ok()) {
		vRRt.mCode = 0;
	}
	return 0;
}
