
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "AgentTcpTask.h"
#include "AgentConfig.h"
#include "SvrCmd.h"

AgentTcpTask::AgentTcpTask(wSocket *socket, int32_t type) : wTcpTask(socket, type) {
	On(CMD_SVR_REQ, SVR_REQ_GXID, &AgentTcpTask::GetSvrByGXid, this);
	On(CMD_SVR_REQ, SVR_REQ_REPORT, &AgentTcpTask::ReportSvr, this);
}

// 客户端查询请求
int AgentTcpTask::GetSvrByGXid(struct Request_t *request) {
	struct SvrReqGXid_t* cmd = reinterpret_cast<struct SvrReqGXid_t*>(request->mBuf);
	AgentConfig* config = Server()->Config<AgentConfig*>();
	
	struct SvrOneRes_t vRRt;
	vRRt.mSvr.mGid = cmd->mGid;
	vRRt.mSvr.mXid = cmd->mXid;
	
	if (config->Qos()->QueryNode(vRRt.mSvr).Ok()) {
		vRRt.mNum = 1;
		AsyncSend(reinterpret_cast<char *>(&vRRt), sizeof(vRRt));
	}
	return 0;
}

// 客户端发来上报请求
int AgentTcpTask::ReportSvr(struct Request_t *request) {
	struct SvrReqReport_t* cmd = reinterpret_cast<struct SvrReqReport_t*>(request->mBuf);
	AgentConfig* config = Server()->Config<AgentConfig*>();
	
	struct SvrResReport_t vRRt;
	if (config->Qos()->CallerNode(cmd->mCaller).Ok()) {
		vRRt.mCode = 1;
		AsyncSend(reinterpret_cast<char *>(&vRRt), sizeof(vRRt));
	}
	return 0;
}
