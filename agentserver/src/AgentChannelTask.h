
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _AGENT_CHANNEL_TASK_H_
#define _AGENT_CHANNEL_TASK_H_

#include "wCore.h"
#include "wStatus.h"
#include "wMisc.h"
#include "wMaster.h"
#include "wChannelTask.h"

using namespace hnet;

class AgentChannelTask : public wChannelTask {
public:
	AgentChannelTask(wSocket *socket, wMaster *master, int32_t type = 0);

	int InitSvrRes(struct Request_t *request);
	int ReloadSvrRes(struct Request_t *request);
	int SyncSvrRes(struct Request_t *request);
};

#endif
