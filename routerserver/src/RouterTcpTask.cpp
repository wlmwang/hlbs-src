
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "RouterTcpTask.h"
#include "SvrCmd.h"

RouterTcpTask(wSocket *socket, int32_t type) : wTcpTask(socket, type) {
	On(CMD_SVR_REQ, SVR_REQ_INIT, &RouterTcpTask::InitSvrReq, this);
	On(CMD_SVR_REQ, SVR_REQ_RELOAD, &RouterTcpTask::ReloadSvrReq, this);
}

// 向单个agent发送init回应
int RouterTcpTask::InitSvrReq(struct Request_t *request) {
	struct SvrResInit_t vRRt;
	vRRt.mCode = 0;
	vRRt.mNum = mConfig->Qos()->GetSvrAll(vRRt.mSvr);
	AsyncSend(reinterpret_cast<char *>(&vRRt), sizeof(vRRt));
	return 0;
}

// 向单个agent发送reload响应
int RouterTcpTask::ReloadSvrReq(struct Request_t *request) {
	struct SvrResReload_t vRRt;
	vRRt.mCode = 0;
	
	mConfig->Qos()->CleanNode();
	mConfig->ParseModSvr(vRRt.mSvr, &vRRt.mNum);
	AsyncSend(reinterpret_cast<char *>(&vRRt), sizeof(vRRt));
	return 0;
}
