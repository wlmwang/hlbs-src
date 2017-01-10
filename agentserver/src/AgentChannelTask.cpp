
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "AgentChannelTask.h"
#include "AgentConfig.h"
#include "SvrCmd.h"

AgentChannelTask::AgentChannelTask(wSocket *socket, wMaster *master, int32_t type) : wChannelTask(socket, master, type) {
	On(CMD_SVR_REQ, SVR_REQ_GXID, &AgentChannelTask::GetSvrByGXid, this);
	On(CMD_SVR_REQ, SVR_REQ_REPORT, &AgentChannelTask::ReportSvr, this);
}

int AgentChannelTask::GetSvrByGXid(struct Request_t *request) {
	struct SvrReqGXid_t* cmd = reinterpret_cast<struct SvrReqGXid_t*>(request->mBuf);
	AgentConfig* config = Config<AgentConfig*>();
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
	AgentConfig* config = Config<AgentConfig*>();

	struct SvrResReport_t vRRt;
	if (config->Qos()->CallerNode(cmd->mCaller).Ok()) {
		vRRt.mCode = 0;
	}
	return 0;
}
