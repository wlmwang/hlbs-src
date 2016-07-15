
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _ROUTER_WORKER_H_
#define _ROUTER_WORKER_H_

#include <map>
#include <vector>

#include "wCore.h"
#include "wLog.h"
#include "wWorker.h"
#include "RouterConfig.h"
#include "RouterServer.h"

class RouterWorker: public wWorker
{
	public:
		RouterWorker(int iSlot = 0) : wWorker(iSlot) {}

		virtual void PrepareRun();
		virtual void Run();

	protected:
		RouterConfig *mConfig {NULL};
		RouterServer *mServer {NULL};
};

#endif