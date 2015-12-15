
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _AGENT_CMD_H_
#define _AGENT_CMD_H_

#include <string>
#include "wType.h"

#include "AgentCmdTask.h"
#include "wTcpClient.h"

#define AGENT_IP   "192.168.8.13"
#define AGENT_PORT 10007

class AgentCmd: public wTcpClient<AgentServerTask>
{
	AgentCmd();
	~AgentCmd();
	
	void PrepareStart(string sIp,unsigned short iPort);
	void Start();
}