
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include <iostream>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "wLog.h"
#include "wMisc.h"
#include "AgentCmd.h"
#include "AgentCmdConfig.h"
#include "RtblCommand.h"


char* CmdGenerator(const char *pText, int iState)
{
	static int iListIdx = 0, iTextLen = 0;
	if(!iState)
	{
		iListIdx = 0;
		iTextLen = strlen(pText);
	}

	const char *pName = NULL;
	while((pName = AgentCmd::Instance()->GetCmdByIndex(iListIdx)))
	{
		iListIdx++;
		if(!strncmp (pName, pText, iTextLen))
		{
			return strdup(pName);
		}
	}
	return NULL;
}

char** CmdCompletion(const char *pText, int iStart, int iEnd)
{
	//rl_attempted_completion_over = 1;
	char **pMatches = NULL;
	if(0 == iStart)
	{
		pMatches = rl_completion_matches(pText, CmdGenerator);
	}

	return pMatches;
}

const char *AgentCmd::GetCmdByIndex(unsigned int iCmdIndex)
{
	if(iCmdIndex >= mCmdMapNum)
	{
		return NULL;
	}
	return mCmdMap[iCmdIndex].pStrCmd;
}

//执行命令
int AgentCmd::Exec(char *pCmdLine)
{
	if(NULL == pCmdLine)
	{
		return -1;
	}

	unsigned int iCmdIndex = 0;
	for(; iCmdIndex < mCmdMapNum; iCmdIndex++)
	{
		if(!strcmp(pCmdLine, mCmdMap[iCmdIndex].pStrCmd))
		{
			break;
		}
	}
	if(mCmdMapNum == iCmdIndex)
	{
		return -1;
	}
	
	//Func_t* pF= mDispatch->GetFuncT(pCmdLine,pCmdLine);
	//pF->mArg1 = pCmdLine;
	//mDispatch.Dispatch(pCmdLine,pCmdLine,pCmdLine);
	mCmdMap[iCmdIndex].vFuncCmd(pCmdLine); //调用相应的函数

	return 0;
}

AgentCmd::AgentCmd() : wTcpClient<AgentCmdTask>(AGENT_SERVER_TYPE, "AgentServer", true)
{
	Initialize();
}

AgentCmd::~AgentCmd() 
{
	//
}

void AgentCmd::RegAct()
{
	mDispatch.RegisterAct("AgentCmd", "GetCmd", REG("GetCmd",&AgentCmd::GetCmd));
	mDispatch.RegisterAct("AgentCmd", "GetCmd", REG("SetCmd",&AgentCmd::SetCmd));
}

void AgentCmd::Initialize()
{
	AgentCmdConfig *pConfig = AgentCmdConfig::Instance();
	mAgentIp = pConfig->mIPAddr;
	mPort = pConfig->mPort;

	if (ConnectToServer(mAgentIp.c_str(), mPort) < 0)
	{
		cout << "Connect to AgentServer failed! Please start it" <<endl;
		LOG_ERROR("default", "Connect to AgentServer failed");
		exit(1);
	}
	
	//初始化定时器
	mClientCheckTimer = wTimer(KEEPALIVE_TIME);
	
	//注册函数回调函数
	mCmdMap.push_back(CMD_ENTRY("GetCmd", &AgentCmd::GetCmd));
	//mCmdMap.push_back(CMD_ENTRY("SetCmd", &AgentCmd::SetCmd));
	//mCmdMap.push_back(CMD_ENTRY("ReloadCmd", &AgentCmd::ReloadCmd));
	//mCmdMap.push_back(CMD_ENTRY("TestEndian", &AgentCmd::TestEndian));
	
	mCmdMap.push_back(CMD_ENTRY_END);
	mCmdMapNum = mCmdMap.size() - 1;

	sprintf(mPrompt, "%s %d>", mAgentIp.c_str(), mPort);
	SetCompletionFunc(CmdCompletion);
}

int AgentCmd::GetCmd(string sCmd)
{
	cout << "mAgentIp" << mAgentIp.c_str() << endl;

	return 0;
}

//准备工作
void AgentCmd::PrepareRun()
{
	//
}

void AgentCmd::Run()
{	
	char *mCmdLine = ReadCmdLine();
	
	if(IsUserQuitCmd(mCmdLine))
	{
		cout << "thanks for used! see you later~" << endl;
		exit(0);
	}
	Exec(mCmdLine);
	
	return;
}

/**
 *  get -a -i 100 -n redis -g 122 -x 122
 *  set -i 100 -d -w 10 -t 122 -c 1222 -s 200
 *  reload agent/router
 *  restart agent/router
 */
int AgentCmd::parseCmd(char *pStr, int iLen)
{
	if(iLen <= 0)
	{
		return -1;
	}
	
	const string DefineCmd[] = {"get", "set", "reload", "restart"};
	
	vector<string> vCmd = Split(pStr," ");
	
	if(DefineCmd[0] == vCmd[0])
	{
		//GetCmd(vCmd[0], vCmd);
	}
	else if(DefineCmd[1] == vCmd[0])
	{
		//SetCmd(vCmd[0], vCmd);
	}
	else if(DefineCmd[2] == vCmd[0])
	{
		//ReloadCmd(vCmd[0], vCmd);
	}
	else if(DefineCmd[3] == vCmd[0])
	{
		//RestartCmd(vCmd[0], vCmd);
	}
	
	return -1;
}

/*
int AgentCmd::GetCmd(string sCmd, vector<string> vParam)
{
	int a = 0, i = 0, g = 0 , x = 0;
	string n = "";
	for(size_t i = 1; i < vParam.size(); i++)
	{
		if(vParam[i] == "-a") {a = 1; continue;}
		if(vParam[i] == "-i") {i = atoi(vParam[i++].c_str()); continue;}
		if(vParam[i] == "-g") {g = atoi(vParam[i++].c_str()); continue;}
		if(vParam[i] == "-x") {x = atoi(vParam[i++].c_str()); continue;}
		if(vParam[i] == "-n") {n = vParam[i++]; continue;}
	}
	
	if(a == 1)
	{
		RtblReqAll_t vRtl;
		return mTcpTask->SyncSend((char *)&vRtl, sizeof(vRtl));
	}
	if(i != 0)
	{
		RtblReqId_t vRtl;
		vRtl.mId = i;
		return mTcpTask->SyncSend((char *)&vRtl, sizeof(vRtl));
	}
	else if(n != "")
	{
		RtblReqName_t vRtl;
		memcpy(vRtl.mName, n.c_str(), n.size());
		return mTcpTask->SyncSend((char *)&vRtl, sizeof(vRtl));
	}
	else if(g != 0 && x == 0)
	{
		RtblReqGid_t vRtl;
		vRtl.mGid = g;
		return mTcpTask->SyncSend((char *)&vRtl, sizeof(vRtl));
	}
	else if(g != 0 && x != 0)
	{
		RtblReqGXid_t vRtl;
		vRtl.mGid = g;
		vRtl.mXid = x;
		return mTcpTask->SyncSend((char *)&vRtl, sizeof(vRtl));
	}
	return -1;
}

int AgentCmd::SetCmd(string sCmd, vector<string> vParam)
{
	//
}

int AgentCmd::ReloadCmd(string sCmd, vector<string> vParam)
{
	//
}

int AgentCmd::RestartCmd(string sCmd, vector<string> vParam)
{
	//
}
*/
