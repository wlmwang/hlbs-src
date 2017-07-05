
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _AGENT_SERVER_H_
#define _AGENT_SERVER_H_

#include "wCore.h"
#include "wMisc.h"
#include "wServer.h"

using namespace hnet;

class AgentServer : public wServer {
public:
	AgentServer(wConfig* config) : wServer(config) { }

	virtual int NewTcpTask(wSocket* sock, wTask** ptr);
	virtual int NewUnixTask(wSocket* sock, wTask** ptr);
	virtual int NewChannelTask(wSocket* sock, wTask** ptr);
};

#endif
