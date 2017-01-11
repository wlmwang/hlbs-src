
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
	RouterConfig* config = Config<RouterConfig*>();

	// 获取所有节点
	int32_t start = 0;
	ssize_t size;
	struct SvrResInit_t vRRt;
	do {
		if (config->Qos()->GetNodeAll(vRRt.mSvr, &vRRt.mNum, start, kMaxNum).Ok() && vRRt.mNum > 0) {
			vRRt.mCode = 0;
			SyncSend(reinterpret_cast<char *>(&vRRt), sizeof(vRRt), &size);
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
	ssize_t size;
	struct SvrResReload_t vRRt;
	do {
		if (config->Qos()->GetNodeAll(vRRt.mSvr, &vRRt.mNum, start, kMaxNum).Ok() && vRRt.mNum > 0) {
			vRRt.mCode = 0;
			SyncSend(reinterpret_cast<char *>(&vRRt), sizeof(vRRt), &size);
		}
		start += vRRt.mNum;
	} while (vRRt.mNum >= kMaxNum);
	return 0;
}
