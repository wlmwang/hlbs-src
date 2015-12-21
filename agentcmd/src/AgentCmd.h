
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

#define REG_FUNC(ActName, vFunc) wDispatch<function<int(string)>>::Func_t {ActName, std::bind(vFunc, this, std::placeholders::_1)}

//#define DEC_DISPATCH(mDispatch) wDispatch<function<int(string)>> mDispatch

#define DEC_FUNC(funcName) int funcName(string sCmd)


/*
char* CmdGenerator(const char *pText, int iState);
char** CmdCompletion(const char *pText, int iStart, int iEnd);
*/

class AgentCmd: public wSingleton<AgentCmd>, public wTcpClient<AgentCmdTask>, public wReadline , public wDispatch<function<int(string)>>
{
	public:
		AgentCmd();

		void Initialize();

		virtual ~AgentCmd();

		virtual void Run();
		virtual void PrepareRun();
		
		int ParseCmd(char *pStr, int iLen);
		
		//const char *GetCmdByIndex(unsigned int iCmdIndex);
		//int Exec(char *pszCmdLine);
		
		void RegAct();
		
		DEC_FUNC(GetCmd);
		
	protected:

		wTimer mClientCheckTimer;
		string mAgentIp;
		unsigned short mPort;
};

#endif