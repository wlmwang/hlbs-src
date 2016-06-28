
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

#define AGENT_HOST "127.0.0.1"
//#define AGENT_HOST "/usr/local/webserver/hlbs/agentserver/log/hlbs.sock"
#define AGENT_PORT 10007
#define AGENT_TIMEOUT 30

#endif