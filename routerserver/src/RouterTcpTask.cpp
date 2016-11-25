
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "RouterTcpTask.h"
#include "RouterConfig.h"
#include "SvrCmd.h"

RouterTcpTask::RouterTcpTask(wSocket *socket, int32_t type) : wTcpTask(socket, type) {
	On(CMD_SVR_REQ, SVR_REQ_INIT, &RouterTcpTask::InitSvrReq, this);
	On(CMD_SVR_REQ, SVR_REQ_RELOAD, &RouterTcpTask::ReloadSvrReq, this);
}

// 向单个agent发送init回应
int RouterTcpTask::InitSvrReq(struct Request_t *request) {
	RouterConfig* config = Server()->Config<RouterConfig*>();

	struct SvrResInit_t vRRt;
	vRRt.mCode = 0;
	config->Qos()->GetNodeAll(vRRt.mSvr, &vRRt.mNum);
	AsyncSend(reinterpret_cast<char *>(&vRRt), sizeof(vRRt));
	return 0;
}

// 向单个agent发送reload响应
int RouterTcpTask::ReloadSvrReq(struct Request_t *request) {
	RouterConfig* config = Server()->Config<RouterConfig*>();

	struct SvrResReload_t vRRt;
	vRRt.mCode = 0;
	config->Qos()->CleanNode();
	config->ParseModifySvr(vRRt.mSvr, &vRRt.mNum);
	AsyncSend(reinterpret_cast<char *>(&vRRt), sizeof(vRRt));
	return 0;
}
