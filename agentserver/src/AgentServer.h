
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _AGENT_SERVER_H_
#define _AGENT_SERVER_H_

#include "wCore.h"
#include "wStatus.h"
#include "wMisc.h"
#include "wServer.h"

using namespace hnet;

class AgentClient;

class AgentServer : public wServer {
public:
	AgentServer(wConfig* config);
	virtual ~AgentServer();

	virtual const wStatus& PrepareRun();
	virtual const wStatus& NewTcpTask(wSocket* sock, wTask** ptr);
	virtual const wStatus& NewChannelTask(wSocket* sock, wTask** ptr);

protected:
	AgentClient* mAgentClient;
};

#endif
