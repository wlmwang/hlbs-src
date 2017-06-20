
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "AgentChannelTask.h"
#include "AgentConfig.h"
#include "SvrCmd.h"

AgentChannelTask::AgentChannelTask(wSocket *socket, wMaster *master, int32_t type) : wChannelTask(socket, master, type) {
	On(CMD_SVR_REQ, SVR_REQ_NTY, &AgentChannelTask::NtySvr, this);
	On(CMD_SVR_REQ, SVR_REQ_REPORT, &AgentChannelTask::ReportSvr, this);
}

int AgentChannelTask::NtySvr(struct Request_t *request) {
	struct SvrReqGXid_t* cmd = reinterpret_cast<struct SvrReqGXid_t*>(request->mBuf);
	Config<AgentConfig*>()->Qos()->NtyNodeSvr(cmd->mSvr);
	return 0;
}

int AgentChannelTask::ReportSvr(struct Request_t *request) {
	struct SvrReqReport_t* cmd = reinterpret_cast<struct SvrReqReport_t*>(request->mBuf);
	Config<AgentConfig*>()->Qos()->CallerNode(cmd->mCaller);
	return 0;
}
