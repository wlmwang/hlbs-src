
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _ROUTER_WORKER_H_
#define _ROUTER_WORKER_H_

#include <map>
#include <vector>

#include "wType.h"
#include "wLog.h"
#include "wWorker.h"
#include "RouterConfig.h"
#include "RouterServer.h"

class RouterWorker: public wWorker
{
	public:
		RouterWorker();
		~RouterWorker();
		void Initialize();

		virtual void PrepareRun();
		virtual void Run();

	protected:
		RouterConfig *mConfig;
		RouterServer *mServer;

		map<long long, string> mTimeEvent;
};

#endif