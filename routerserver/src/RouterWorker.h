
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _ROUTER_WORKER_H_
#define _ROUTER_WORKER_H_

#include <map>
#include <vector>

#include "wType.h"
//#include "wConfig.h"
//#include "wLog.h"
//#include "Svr.h"
#include "wWorker.h"
#include "RouterMaster.h"
#include "RouterServer.h"
#include "RouterConfig.h"

class RouterWorker: public wWorker
{
	public:
		RouterWorker();
		~RouterWorker();
		void Initialize();

		virtual void PrepareRun();
		virtual void Run();
};

#endif