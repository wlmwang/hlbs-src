
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "AgentUnixTask.h"
#include "AgentConfig.h"
#include "SvrCmd.h"

AgentUnixTask::AgentUnixTask(wSocket *socket, int32_t type) : wUnixTask(socket, type) {
	On(CMD_SVR_REQ, SVR_REQ_GXID, &AgentUnixTask::GetSvrByGXid, this);
	On(CMD_SVR_REQ, SVR_REQ_REPORT, &AgentUnixTask::ReportSvr, this);
}

// 客户端查询请求
int AgentUnixTask::GetSvrByGXid(struct Request_t *request) {
	struct SvrReqGXid_t* cmd = reinterpret_cast<struct SvrReqGXid_t*>(request->mBuf);
	AgentConfig* config = Config<AgentConfig*>();

	struct SvrOneRes_t vRRt;
	vRRt.mSvr = cmd->mSvr;
	if (config->Qos()->QueryNode(vRRt.mSvr).Ok()) {
		vRRt.mCode = 0;
		vRRt.mNum = 1;
		AsyncSend(reinterpret_cast<char*>(&vRRt), sizeof(vRRt));

		// 同步其他worker进程调用数
		//struct SvrReqNty_t vRRt1;
		//vRRt1.mSvr = vRRt.mSvr;
		//AsyncWorker(reinterpret_cast<char*>(&vRRt1), sizeof(vRRt1));
	} else {
		vRRt.mCode = -1;
		vRRt.mNum = 0;
		AsyncSend(reinterpret_cast<char*>(&vRRt), sizeof(vRRt));
	}
	return 0;
}

// 客户端发来上报请求
int AgentUnixTask::ReportSvr(struct Request_t *request) {
	struct SvrReqReport_t* cmd = reinterpret_cast<struct SvrReqReport_t*>(request->mBuf);
	AgentConfig* config = Config<AgentConfig*>();

	struct SvrResReport_t vRRt;
	if (config->Qos()->CallerNode(cmd->mCaller).Ok()) {
		vRRt.mCode = 0;
		AsyncSend(reinterpret_cast<char*>(&vRRt), sizeof(vRRt));

		// 同步其他worker进程
		//AsyncWorker(reinterpret_cast<char*>(cmd), request->mLen);
	} else {
		vRRt.mCode = -1;
		AsyncSend(reinterpret_cast<char*>(&vRRt), sizeof(vRRt));
	}
	return 0;
}
