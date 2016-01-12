
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
#include "wReadline.h"
#include "wDispatch.h"
#include "AgentCmdTask.h"
#include "ReadlineThread.h"
#include "wMTcpClient.h"
#include "SvrCommand.h"
#include "wShareMemory.h"
#include "wMsgQueue.h"

#define REG_FUNC_S(name, vFunc) wDispatch<function<int(string, vector<string>)>, string>::Func_t {name, std::bind(vFunc, this, std::placeholders::_1, std::placeholders::_2)}
#define DEC_DISP_S(dispatch) wDispatch<function<int(string, vector<string>)>, string> dispatch
#define DEC_FUNC_S(func) int func(string sCmd, vector<string>)

#define CMD_REG_DISP_S(name, func) mDispatch.Register("AgentCmd", name, REG_FUNC_S(name, func));

class AgentCmd: public wSingleton<AgentCmd>, public wMTcpClient<AgentCmdTask>
{
	public:
		AgentCmd();

		void Initialize();
		void InitShareMemory();

		virtual ~AgentCmd();

		virtual void Run();
		virtual void PrepareRun();

		void CheckCmd();
		int ParseCmd(char *pStr, int iLen);

		DEC_FUNC_S(GetCmd);
		DEC_FUNC_S(SetCmd);
		DEC_FUNC_S(ReportCmd);
		DEC_FUNC_S(ReloadCmd);
		DEC_FUNC_S(RestartCmd);

		void SetWaitResStatus(bool bStatus = true)
		{
			mWaitResStatus = bStatus;
		}
		
		bool IsWaitResStatus()
		{
			return mWaitResStatus == true;
		}

		wTcpTask* TcpTask()
		{
			wTcpClient<AgentCmdTask>* pClient = OneTcpClient(SERVER_AGENT);
			if(pClient != NULL && pClient->TcpTask())
			{
				return pClient->TcpTask();
			}
			return 0;
		}

		int SyncSend(const char *pCmd, int iLen)
		{
			wTcpTask *pTcpTask = TcpTask();
			if(pTcpTask != 0)
			{
				return pTcpTask->SyncSend(pCmd, iLen);
			}
			return -1;
		}

	protected:
		DEC_DISP_S(mDispatch);
		unsigned long long mTicker;
		bool mWaitResStatus;
		ReadlineThread *mReadlineThread;

		wShareMemory *mInShareMem;	//输入的消息队列的缓冲区位置
		wShareMemory *mOutShareMem; //输出的消息队列的缓冲区位置
		wMsgQueue* mInMsgQueue;	// 输入的消息队列
		wMsgQueue* mOutMsgQueue;// 输出的消息队列
};

#endif