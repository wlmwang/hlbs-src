
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _ROUTER_TCP_TASK_H_
#define _ROUTER_TCP_TASK_H_

#include "wCore.h"
#include "wMisc.h"
#include "wTcpTask.h"

using namespace hnet;

class RouterTcpTask : public wTcpTask {
public:
	RouterTcpTask(wSocket *socket, int32_t type);
	
    virtual int Connect();
    virtual int DisConnect();

	int InitSvrReq(struct Request_t *request);
	int ReloadSvrReq(struct Request_t *request);

	int InitAgntReq(struct Request_t *request);
	int ReloadAgntReq(struct Request_t *request);
	int SyncAgntRes(struct Request_t *request);
};

#endif
