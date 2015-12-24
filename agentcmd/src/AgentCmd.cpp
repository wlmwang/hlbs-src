
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

AgentCmd::AgentCmd() : wTcpClient<AgentCmdTask>(AGENT_SERVER_TYPE, "AgentServer", true)
{
	Initialize();
}

AgentCmd::~AgentCmd() 
{
	//
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

	char cStr[32];
	sprintf(cStr, "%s %d>", mAgentIp.c_str(), mPort);
	mReadline.SetPrompt(cStr, strlen(cStr));
	
	mReadline.SetCompletion(&AgentCmd::Completion);
	
	mDispatch.Register("AgentCmd", "GetCmd", REG_FUNC("GetCmd",&AgentCmd::GetCmd));
	mDispatch.Register("AgentCmd", "SetCmd", REG_FUNC("SetCmd",&AgentCmd::SetCmd));
	mDispatch.Register("AgentCmd", "ReloadCmd", REG_FUNC("ReloadCmd",&AgentCmd::ReloadCmd));
	mDispatch.Register("AgentCmd", "RestartCmd", REG_FUNC("RestartCmd",&AgentCmd::RestartCmd));

	mLastTicker = GetTickCount();
	mWaitResTimer = wTimer(WAITRES_TIME);
}

char* AgentCmd::Generator(const char *pText, int iState)
{
	static int iListIdx = 0, iTextLen = 0;
	if(!iState)
	{
		iListIdx = 0;
		iTextLen = strlen(pText);
	}
	
	const char *pName = NULL;
	/*
	while((pName = AgentCmd::Instance()->GetCmdByIndex(iListIdx)))
	{
		iListIdx++;
		if(!strncmp (pName, pText, iTextLen))
		{
			return strdup(pName);
		}
	}
	*/
	return NULL;
}

char** AgentCmd::Completion(const char *pText, int iStart, int iEnd)
{
	//rl_attempted_completion_over = 1;
	char **pMatches = NULL;
	if(0 == iStart)
	{
		pMatches = rl_completion_matches(pText, &AgentCmd::Generator);
	}
	return pMatches;
}

//准备工作
void AgentCmd::PrepareRun()
{
	//
}

void AgentCmd::Run()
{
	ReadCmdLine();

	//CheckTimer();
	
	return;
}

void AgentCmd::ReadCmdLine()
{
	unsigned long long iInterval = (unsigned long long)(GetTickCount() - mLastTicker);
	if(iInterval < 100) 	//100ms
	{
		return;
	}

	if(IsWaitResStatus() && mWaitResTimer.CheckTimer(iInterval))
	{
		cout << "command executed timeout" << endl;
	}
	else if(IsWaitResStatus() && !mWaitResTimer.CheckTimer(iInterval))
	{
		return;
	}
	
	//read cmd
	char *pCmdLine = mReadline.ReadCmdLine();
	if(mReadline.IsUserQuitCmd(pCmdLine))
	{
		cout << "thanks for used! see you later~" << endl;
		exit(0);
	}

	ParseCmd(pCmdLine, strlen(pCmdLine));
}

int AgentCmd::ParseCmd(char *pCmdLine, int iLen)
{
	if(0 == iLen)
	{
		return -1;
	}
	
	vector<string> vToken = Split(pCmdLine, " ");
	
	if (vToken.size() > 0 && vToken[0] != "" && vToken[1] != "")
	{
		auto pF = mDispatch.GetFuncT("AgentCmd", vToken[0]);

		if (pF != NULL)
		{
			pF->mFunc(vToken[1],vToken);
		}
	}
	
	return -1;
}

//get -a -i 100 -n redis -g 122 -x 122
int AgentCmd::GetCmd(string sCmd, vector<string> vParam)
{
	int a = 0, i = 0, g = 0 , x = 0;
	string n = "";
	for(size_t i = 1; i < vParam.size(); i++)
	{
		if(vParam[i] == "-a") {a = 1; continue;}
		else if(vParam[i] == "-i") {i = atoi(vParam[++i].c_str()); continue;}
		else if(vParam[i] == "-g") {g = atoi(vParam[++i].c_str()); continue;}
		else if(vParam[i] == "-x") {x = atoi(vParam[++i].c_str()); continue;}
		else if(vParam[i] == "-n") {n = vParam[++i]; continue;}
		else { cout << "Unknow param.";}
	}

	SetWaitResStatus();
	if(a == 1)
	{
		RtblReqAll_t vRtl;
		return mTcpTask->SyncSend((char *)&vRtl, sizeof(vRtl));
	}
	else if(i != 0)
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
	SetWaitResStatus(false);
	return -1;
}

//set -i 100 -d -w 10 -t 122 -c 1222 -s 200 -k 500
int AgentCmd::SetCmd(string sCmd, vector<string> vParam)
{
	RtblSetReqId_t stSetRtbl;
	for(size_t i = 1; i < vParam.size(); i++)
	{
		if(vParam[i] == "-d") {stSetRtbl.mDisabled = 1; continue;}
		else if(vParam[i] == "-i") {stSetRtbl.mId = atoi(vParam[++i].c_str()); continue;}
		else if(vParam[i] == "-w") {stSetRtbl.mWeight = atoi(vParam[++i].c_str()); continue;}
		else if(vParam[i] == "-k") {stSetRtbl.mTasks = atoi(vParam[++i].c_str()); continue;}
		else if(vParam[i] == "-t") {stSetRtbl.mTimeline = atoi(vParam[++i].c_str()); continue;}
		else if(vParam[i] == "-c") {stSetRtbl.mConnTime = atoi(vParam[++i].c_str()); continue;}
		else if(vParam[i] == "-s") {stSetRtbl.mSuggest = atoi(vParam[++i].c_str()); continue;}
		else { cout << "Unknow param.";}
	}
	if(stSetRtbl.mId == 0)
	{
		cout << "need -i param" << endl;
	}
	else
	{
		SetWaitResStatus();
		return mTcpTask->SyncSend((char *)&stSetRtbl, sizeof(stSetRtbl));
	}
	return -1;
}

//reload
int AgentCmd::ReloadCmd(string sCmd, vector<string> vParam)
{
	SetWaitResStatus();
	RtblReqReload_t vRtl;
	return mTcpTask->SyncSend((char *)&vRtl, sizeof(vRtl));
}

//restart agent/router
int AgentCmd::RestartCmd(string sCmd, vector<string> vParam)
{
	//
	return -1;
}
