
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _COMMON_H_
#define _COMMON_H_

enum CLIENT_TYPE
{
	CLIENT_USER = 2,
	SERVER_ROUTER,
	SERVER_AGENT,
	SERVER_CMD,
};

#define ROUTER_LOGIN false
#define AGENT_LOGIN false

#define AGENT_SHM "/tmp/report-agent.bin"

#endif