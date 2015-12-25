
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

#define REG_FUNC_S(name, vFunc) wDispatch<function<int(string, vector<string>)>, string>::Func_t {name, std::bind(vFunc, this, std::placeholders::_1, std::placeholders::_2)}
#define DEC_DISP_S(dispatch) wDispatch<function<int(string, vector<string>)>, string> dispatch
#define DEC_FUNC_S(func) int func(string sCmd, vector<string>)

#define CMD_REG_DISP_S(name, func) mDispatch.Register("AgentCmd", name, REG_FUNC_S(name, func));

class AgentCmd: public wSingleton<AgentCmd>, public wTcpClient<AgentCmdTask>
{
	public:
		AgentCmd();

		void Initialize();

		virtual ~AgentCmd();

		virtual void Run();
		virtual void PrepareRun();
		
		void ReadCmdLine();
		int ParseCmd(char *pStr, int iLen);
		
		DEC_FUNC_S(GetCmd);
		DEC_FUNC_S(SetCmd);
		DEC_FUNC_S(ReloadCmd);
		DEC_FUNC_S(RestartCmd);

		static char* Generator(const char *pText, int iState);
		static char** Completion(const char *pText, int iStart, int iEnd);
		
		void SetWaitResStatus(bool bStatus = true)
		{
			mWaitResStatus = bStatus;
		}
		
		bool IsWaitResStatus()
		{
			return mWaitResStatus == true;
		}

	protected:
		
		DEC_DISP_S(mDispatch);
		wReadline mReadline;
		string mAgentIp;
		unsigned short mPort;
		
		unsigned long long mTicker;
		bool mWaitResStatus;
};

#endif