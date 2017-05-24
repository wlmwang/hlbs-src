
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _ROUTER_HTTP_TASK_H_
#define _ROUTER_HTTP_TASK_H_

#include "wCore.h"
#include "wStatus.h"
#include "wMisc.h"
#include "wHttpTask.h"

using namespace hnet;

const uint8_t CMD_SVR_HTTP = 70;

// 添加&更新SVR记录
const uint8_t SVR_HTTP_SAVE = 0;

// 重载所有SVR记录
const uint8_t SVR_HTTP_INIT = 1;

class RouterHttpTask : public wHttpTask {
public:
	RouterHttpTask(wSocket *socket, int32_t type);

	int SaveSvrReq(struct Request_t *request);
	int InitSvrReq(struct Request_t *request);
};

#endif
