
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _ROUTER_MASTER_H_
#define _ROUTER_MASTER_H_

#include <map>
#include <vector>

#include "wType.h"
#include "wMaster.h"
#include "RouterWorker.h"

static char *master_process = "master process ";

class RouterMaster : public wMaster<RouterMaster>
{
	public:
		RouterMaster();
		virtual ~RouterMaster();
		void Initialize();
		
		virtual void PrepareRun();
		virtual void Run();
		
		virtual wWorker* NewWorker(int iSlot = 0);
	
	protected:
		char *mTitle;
};

#endif