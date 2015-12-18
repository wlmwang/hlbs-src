
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _AGENT_CMD_H_
#define _AGENT_CMD_H_

#include <string>
#include <vector>

#include "wType.h"
#include "wTimer.h"

#include "wSingleton.h"
#include "AgentCmdTask.h"
#include "wTcpClient.h"
#include "wReadline.h"

#define AGENT_SERVER_TYPE 1

typedef int (*CmdProcFunc)(string sCmd, vector<string> vParam);
typedef struct{
	const char   *pszCmd;
	CmdProcFunc  fpCmd;
} CMD_PROC;


#define CMD_ENTRY(cmdStr, func) {cmdStr, func}
#define CMD_ENTRY_END  {NULL, NULL}

//命令处理函数定义
#define MOCK_FUNC(funcName) \
	int funcName(string sCmd, vector<string> vParam)

char* CmdGenerator(const char *pText, int iState);
char** CmdCompletion(const char *pText, int iStart, int iEnd);

const char *GetCmdByIndex(unsigned int dwCmdIndex);
int ExecCmd(char *pszCmdLine);

MOCK_FUNC(GetCmd);

class AgentCmd: public wSingleton<AgentCmd>, public wTcpClient<AgentCmdTask>, public wReadline
{
	public:
		AgentCmd();

		void Initialize();

		virtual ~AgentCmd();

		virtual void Run();
		virtual void PrepareRun();
		
		int parseCmd(char *pStr, int iLen);
		
		//int GetCmd(string sCmd, vector<string> vParam);
		//int SetCmd(string sCmd, vector<string> vParam);
		//int ReloadCmd(string sCmd, vector<string> vParam);
		//int RestartCmd(string sCmd, vector<string> vParam);
	
	protected:
		wTimer mClientCheckTimer;
		string mAgentIp;
		unsigned short mPort;
};

#endif