
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "RouterTcpTask.h"
#include "RouterConfig.h"
#include "wLogger.h"
#include "SvrCmd.h"

RouterTcpTask::RouterTcpTask(wSocket *socket, int32_t type) : wTcpTask(socket, type) {
	On(CMD_SVR_REQ, SVR_REQ_INIT, &RouterTcpTask::InitSvrReq, this);
	On(CMD_SVR_REQ, SVR_REQ_RELOAD, &RouterTcpTask::ReloadSvrReq, this);
}

// 向单个agent发送init回应
int RouterTcpTask::InitSvrReq(struct Request_t *request) {
	RouterConfig* config = Server()->Config<RouterConfig*>();

	struct SvrResInit_t vRRt;
	// 获取所有节点
	if (config->Qos()->GetNodeAll(vRRt.mSvr, &vRRt.mNum).Ok()) {
		vRRt.mCode = 0;
		AsyncSend(reinterpret_cast<char *>(&vRRt), sizeof(vRRt));
	}
	return 0;
}

// 向单个agent发送reload响应
int RouterTcpTask::ReloadSvrReq(struct Request_t *request) {
	RouterConfig* config = Server()->Config<RouterConfig*>();

	// 清除node节点
	config->Qos()->CleanNode();

	struct SvrResReload_t vRRt;
	// 重新读取svr.xml文件
	if (config->ParseModifySvr(vRRt.mSvr, &vRRt.mNum).Ok()) {
		vRRt.mCode = 0;
		AsyncSend(reinterpret_cast<char *>(&vRRt), sizeof(vRRt));
	}
	return 0;
}
