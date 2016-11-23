
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _ROUTER_TCP_TASK_H_
#define _ROUTER_TCP_TASK_H_

#include "wCore.h"
#include "wStatus.h"
#include "wMisc.h"
#include "wTcpTask.h"

using namespace hnet;

class RouterTcpTask : public wTcpTask {
public:
	RouterTcpTask(wSocket *socket, int32_t type);

	int InitSvrReq(struct Request_t *request);
	int ReloadSvrReq(struct Request_t *request);
};

#endif
