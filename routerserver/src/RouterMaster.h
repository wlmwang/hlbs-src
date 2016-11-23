
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _ROUTER_MASTER_H_
#define _ROUTER_MASTER_H_

#include "wCore.h"
#include "wMaster.h"

class RouterMaster : public wMaster {
public:
	virtual const wStatus& PrepareRun();
	virtual const wStatus& Reload();
};

#endif
