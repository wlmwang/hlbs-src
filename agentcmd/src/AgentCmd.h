
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _AGENT_CMD_H_
#define _AGENT_CMD_H_

#include "wType.h"
#include "wTimer.h"

#include "wSingleton.h"
#include "AgentCmdTask.h"
#include "wTcpClient.h"

#define AGENT_SERVER_TYPE 1

class AgentCmd: public wSingleton<AgentCmd>, public wTcpClient<AgentCmdTask>
{
	public:
		AgentCmd();

		void Initialize();

		virtual ~AgentCmd();

		virtual void Run();
		virtual void PrepareRun();
		
		int parseCmd(char *pStr, int iLen);
		
		int GetCmd(string sCmd, vector<string> vParam);
		int SetCmd(string sCmd, vector<string> vParam);
		int ReloadCmd(string sCmd, vector<string> vParam);
		int RestartCmd(string sCmd, vector<string> vParam);
	protected:
		wTimer mClientCheckTimer;
		string mAgentIp;
		unsigned short mPort;
};

#endif