
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

#define CMD_ENTRY(cmdStr, func) {cmdStr, std::bind(func, this, std::placeholders::_1)}
#define CMD_ENTRY_END  {NULL, NULL}

#define REG(ActName, vFunc) Func_t<function<int(string)>, string>{ActName, std::bind(vFunc, this, std::placeholders::_1), 1}


#define CMD_FUNC(funcName) int funcName(string sCmd)

char* CmdGenerator(const char *pText, int iState);
char** CmdCompletion(const char *pText, int iStart, int iEnd);

class AgentCmd: public wSingleton<AgentCmd>, public wTcpClient<AgentCmdTask>, public wReadline
{
	public:
		AgentCmd();

		void Initialize();

		virtual ~AgentCmd();

		virtual void Run();
		virtual void PrepareRun();
		
		int parseCmd(char *pStr, int iLen);
		
		CMD_FUNC(GetCmd);
		
		const char *GetCmdByIndex(unsigned int iCmdIndex);
		int Exec(char *pszCmdLine);
		
		void RegAct();
	protected:

		typedef struct
		{
			const char  *pStrCmd;
			function<int(string)> vFuncCmd;
		} CmdProc_t;

		wDispatch mDispatch;
		
		vector<CmdProc_t> mCmdMap;
		int mCmdMapNum;

		wTimer mClientCheckTimer;
		string mAgentIp;
		unsigned short mPort;
};

#endif