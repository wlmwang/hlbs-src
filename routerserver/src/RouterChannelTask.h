
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _ROUTER_CHANNEL_TASK_H_
#define _ROUTER_CHANNEL_TASK_H_

#include "wCore.h"
#include "wMisc.h"
#include "wMaster.h"
#include "wChannelTask.h"

using namespace hnet;

class RouterChannelTask : public wChannelTask {
public:
	RouterChannelTask(wSocket *socket, wMaster *master, int32_t type = 0);

	int ReloadSvrRes(struct Request_t *request);
	int SyncSvrRes(struct Request_t *request);

	int ReloadAgntRes(struct Request_t *request);
	int SyncAgntRes(struct Request_t *request);
};

#endif
