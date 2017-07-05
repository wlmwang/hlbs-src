
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _ROUTER_MASTER_H_
#define _ROUTER_MASTER_H_

#include "wCore.h"
#include "wMaster.h"

using namespace hnet;

class RouterMaster : public wMaster {
public:
	RouterMaster(const std::string& title, wServer* server) : wMaster(title, server) { }

	virtual int PrepareRun();
	virtual int Reload();
};

#endif
