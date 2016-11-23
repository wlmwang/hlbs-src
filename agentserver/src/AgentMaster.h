
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _AGENT_MASTER_H_
#define _AGENT_MASTER_H_

#include "wCore.h"
#include "wMaster.h"

class AgentMaster : public wMaster {
public:
	virtual const wStatus& PrepareRun();
	virtual const wStatus& Reload();
};

#endif
