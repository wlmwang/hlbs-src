
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "wLogger.h"
#include "RouterHttpTask.h"
#include "RouterConfig.h"
#include "SvrCmd.h"

RouterHttpTask::RouterHttpTask(wSocket *socket, int32_t type) : wHttpTask(socket, type) {
	On(CMD_SVR_HTTP, SVR_HTTP_SAVE, &RouterHttpTask::SaveSvrReq, this);
	On(CMD_SVR_HTTP, SVR_HTTP_INIT, &RouterHttpTask::InitSvrReq, this);
}

int RouterHttpTask::SaveSvrReq(struct Request_t *request) {
	RouterConfig* config = Config<RouterConfig*>();
	// json

	config->WriteSvrConfFile();

	// 返回
	ResponseSet("Content-Type", "application/json; charset=UTF-8");
	Write("{'status': 200, 'msg': 'ok'}");
	return 0;
}

int RouterHttpTask::InitSvrReq(struct Request_t *request) {
	return 0;
}