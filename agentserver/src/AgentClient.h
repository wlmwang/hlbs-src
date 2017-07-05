
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _AGENT_CLIENT_H_
#define _AGENT_CLIENT_H_

#include "wCore.h"
#include "wMultiClient.h"

using namespace hnet;

const int kType = 0;

class AgentClient : public wMultiClient {
public:
	AgentClient(wConfig* config, wServer* server = NULL): wMultiClient(config, server, true), mPort(0), mInitClient(false) { }

	virtual int PrepareRun();
	virtual int Run();
	virtual int NewTcpTask(wSocket* sock, wTask** ptr, int type = 0);

protected:
	std::string mHost;
	uint16_t mPort;

	bool mInitClient;
};

#endif
