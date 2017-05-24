
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "wLogger.h"
#include "RouterTcpTask.h"
#include "RouterConfig.h"
#include "SvrCmd.h"

RouterTcpTask::RouterTcpTask(wSocket *socket, int32_t type) : wTcpTask(socket, type) {
	On(CMD_SVR_REQ, SVR_REQ_INIT, &RouterTcpTask::InitSvrReq, this);
	On(CMD_SVR_REQ, SVR_REQ_RELOAD, &RouterTcpTask::ReloadSvrReq, this);
}

// 向单个agent发送init回应
int RouterTcpTask::InitSvrReq(struct Request_t *request) {
	RouterConfig* config = Config<RouterConfig*>();

	// 获取所有节点
	int32_t start = 0;
	struct SvrResInit_t vRRt;
	do {
		if (config->Qos()->GetNodeAll(vRRt.mSvr, &vRRt.mNum, start, kMaxNum).Ok() && vRRt.mNum > 0) {
			vRRt.mCode = 0;
			AsyncSend(reinterpret_cast<char *>(&vRRt), sizeof(vRRt));
		}
		start += vRRt.mNum;
	} while (vRRt.mNum >= kMaxNum);
	return 0;
}

// 向单个agent发送reload响应
int RouterTcpTask::ReloadSvrReq(struct Request_t *request) {
	RouterConfig* config = Config<RouterConfig*>();

	// 重新加载配置
	config->Qos()->CleanNode();
	config->ParseSvrConf();
	
	int32_t start = 0;
	struct SvrResReload_t vRRt;
	do {
		if (config->Qos()->GetNodeAll(vRRt.mSvr, &vRRt.mNum, start, kMaxNum).Ok() && vRRt.mNum > 0) {
			vRRt.mCode = 0;
			AsyncSend(reinterpret_cast<char *>(&vRRt), sizeof(vRRt));
		}
		start += vRRt.mNum;
	} while (vRRt.mNum >= kMaxNum);
	return 0;
}
