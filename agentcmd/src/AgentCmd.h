
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _AGENT_CMD_H_
#define _AGENT_CMD_H_

#include <string>
#include <vector>
#include <functional>

#include "wType.h"
#include "wTimer.h"

#include "wSingleton.h"
#include "AgentCmdTask.h"
#include "wTcpClient.h"
#include "wReadline.h"
#include "wDispatch.h"

#define AGENT_SERVER_TYPE 1

#define REG_FUNC(ActName, vFunc) wDispatch<function<int(string, vector<string>)> >::Func_t {ActName, std::bind(vFunc, this, std::placeholders::_1, std::placeholders::_2)}

#define DEC_FUNC(funcName) int funcName(string sCmd, vector<string>)

class AgentCmd: public wSingleton<AgentCmd>, public wTcpClient<AgentCmdTask>, public wDispatch<function<int(string, vector<string>)> >
{
	public:
		AgentCmd();

		void Initialize();

		virtual ~AgentCmd();

		virtual void Run();
		virtual void PrepareRun();
		
		int ParseCmd(char *pStr, int iLen);

		void RegAct();
		
		DEC_FUNC(GetCmd);
		DEC_FUNC(SetCmd);
		DEC_FUNC(ReloadCmd);
		DEC_FUNC(RestartCmd);

		static char* Generator(const char *pText, int iState);
		static char** Completion(const char *pText, int iStart, int iEnd);
	
	protected:
	
		wReadline mReadline;
		wTimer mClientCheckTimer;
		string mAgentIp;
		unsigned short mPort;
};

#endif