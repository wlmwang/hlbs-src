
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _ROUTER_MASTER_H_
#define _ROUTER_MASTER_H_

#include <map>
#include <vector>

#include "wCore.h"
#include "wMaster.h"
#include "RouterConfig.h"
#include "RouterServer.h"

class RouterMaster : public wMaster<RouterMaster>
{
	public:
		virtual ~RouterMaster();

		virtual void PrepareRun();
		virtual wWorker* NewWorker(int iSlot = 0);
		virtual void ReloadMaster();

	protected:
		char *mTitle {NULL};
		RouterConfig *mConfig {NULL};
		RouterServer *mServer {NULL};
};

#endif
