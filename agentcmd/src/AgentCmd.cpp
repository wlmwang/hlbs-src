
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

AgentCmd::AgentCmd()
{
	mInShareMem = 0;
	mOutShareMem = 0;
	mInMsgQueue = 0;
	mOutMsgQueue = 0;
	Initialize();
}

AgentCmd::~AgentCmd() 
{
	SAFE_DELETE(mInShareMem);
	SAFE_DELETE(mOutShareMem);
	SAFE_DELETE(mInMsgQueue);
	SAFE_DELETE(mOutMsgQueue);
}

void AgentCmd::Initialize()
{
	mTicker = GetTickCount();
	SetWaitResStatus(false);
	InitShareMemory();

	CMD_REG_DISP_S("get", &AgentCmd::GetCmd);
	CMD_REG_DISP_S("set", &AgentCmd::SetCmd);
	CMD_REG_DISP_S("report", &AgentCmd::ReportCmd);
	CMD_REG_DISP_S("reload", &AgentCmd::ReloadCmd);
	CMD_REG_DISP_S("restart", &AgentCmd::RestartCmd);
}

void AgentCmd::InitShareMemory()
{
	mInShareMem = new wShareMemory(SVR_SHARE_MEM_PIPE, 'o', MSG_QUEUE_LEN);
	mOutShareMem = new wShareMemory(SVR_SHARE_MEM_PIPE, 'i', MSG_QUEUE_LEN);
	char * pBuff = NULL;
	if((pBuff = mInShareMem->AttachShareMemory()) != NULL)
	{
		mInMsgQueue = new wMsgQueue();
		mInMsgQueue->SetBuffer(pBuff, MSG_QUEUE_LEN);
	}
	else
	{
		LOG_ERROR("error","[startup] Attach (In) Share Memory failed");
	}
	if((pBuff = mOutShareMem->AttachShareMemory()) != NULL)
	{
		mOutMsgQueue = new wMsgQueue();
		mOutMsgQueue->SetBuffer(pBuff, MSG_QUEUE_LEN);
	}
	else
	{
		LOG_ERROR("error","[startup] Attach (Out) Share Memory failed");
	}
}

//准备工作
void AgentCmd::PrepareRun()
{
	AgentCmdConfig *pConfig = AgentCmdConfig::Instance();
	
	bool bRet = GenerateClient(SERVER_AGENT, "SERVER AGENT", pConfig->mIPAddr, pConfig->mPort);
	if (!bRet)
	{
		cout << "Connect to AgentServer failed! Please start it" <<endl;
		LOG_ERROR("error", "[startup] Connect to AgentServer failed");
		exit(1);
	}

	char cStr[32];
	sprintf(cStr, "%s %d>", pConfig->mIPAddr, pConfig->mPort);
	mReadlineThread = new ReadlineThread(cStr, strlen(cStr));

	mReadlineThread->CreateThread();
}

void AgentCmd::Run()
{
	CheckCmd();
}

void AgentCmd::CheckCmd()
{
	unsigned long long iInterval = (unsigned long long)(GetTickCount() - mTicker);
	
	if(IsWaitResStatus() && iInterval > WAITRES_TIME)
	{
		cout << "command executed timeout" << endl;
	}
	else if(IsWaitResStatus() && iInterval <= WAITRES_TIME)
	{
		return;
	}
	
	mTicker += iInterval;
	
	mReadlineThread->WakeUp();	//唤醒readline线程，进入等待输入状态

	if (mReadlineThread->mCmdLine != 0)
	{
		ParseCmd(mReadlineThread->mCmdLine, strlen(mReadlineThread->mCmdLine));
		mReadlineThread->mCmdLine = 0;
	}
	else if(mReadlineThread->IsStop())
	{
		cout << "thanks for used! see you later~" << endl;
		exit(0);
	}
}

int AgentCmd::ParseCmd(char *pCmdLine, int iLen)
{
	if(0 == iLen)
	{
		return -1;
	}
	
	vector<string> vToken = Split(pCmdLine, " ");
	
	if (vToken.size() > 0 && vToken[0] != "")
	{
		auto pF = mDispatch.GetFuncT("AgentCmd", vToken[0]);

		if (pF != NULL)
		{
			if (vToken.size() == 1)
			{
				vToken.push_back("");
			}
			return pF->mFunc(vToken[1],vToken);
		}
	}
	
	return -1;
}

//get [-a] [-i 100] [-n redis] [-g 122 -x 122]
int AgentCmd::GetCmd(string sCmd, vector<string> vParam)
{
	int a = 0, i = 0, g = 0 , x = 0;
	string n = "";
	int iCnt = vParam.size();
	for(size_t j = 1; j < vParam.size(); j++)
	{
		if(vParam[j] == "-a") 
		{
			a = 1; continue;
		}
		else if(vParam[j] == "-i" && j + 1 < iCnt) 
		{
			i = atoi(vParam[++j].c_str()); continue;
		}
		else if(vParam[j] == "-g" && j + 1 < iCnt) 
		{
			g = atoi(vParam[++j].c_str()); continue;
		}
		else if(vParam[j] == "-x" && j + 1 < iCnt) 
		{
			x = atoi(vParam[++j].c_str()); continue;
		}
		else if(vParam[j] == "-n" && j + 1 < iCnt) 
		{
			n = vParam[++j]; continue;
		}
		else 
		{
			cout << "Unknow param." << endl;
			return -1;
		}
	}

	SetWaitResStatus();
	if(a == 1)
	{
		SvrReqAll_t vRtl;
		return SyncSend((char *)&vRtl, sizeof(vRtl));
	}
	else if(i != 0)
	{
		SvrReqId_t vRtl;
		vRtl.mId = i;
		return SyncSend((char *)&vRtl, sizeof(vRtl));
	}
	else if(n != "")
	{
		SvrReqName_t vRtl;
		memcpy(vRtl.mName, n.c_str(), n.size());
		return SyncSend((char *)&vRtl, sizeof(vRtl));
	}
	else if(g != 0 && x == 0)
	{
		SvrReqGid_t vRtl;
		vRtl.mGid = g;
		return SyncSend((char *)&vRtl, sizeof(vRtl));
	}
	else if(g != 0 && x != 0)
	{
		SvrReqGXid_t vRtl;
		vRtl.mGid = g;
		vRtl.mXid = x;
		return SyncSend((char *)&vRtl, sizeof(vRtl));
	}
	SetWaitResStatus(false);
	return -1;
}

//set -i 100 [-d 1] [-w 10] [-t 122] [-c 1222] [-s 200] [-k 500]
int AgentCmd::SetCmd(string sCmd, vector<string> vParam)
{
	SvrSetReqId_t stSetRtbl;
	int iCnt = vParam.size();
	for(size_t j = 1; j < vParam.size(); j++)
	{
		if(vParam[j] == "-d" && j + 1 < iCnt) 
		{
			stSetRtbl.mDisabled = atoi(vParam[++j].c_str()); continue;
		}
		else if(vParam[j] == "-i" && j + 1 < iCnt) 
		{
			stSetRtbl.mId = atoi(vParam[++j].c_str()); continue;
		}
		else if(vParam[j] == "-w" && j + 1 < iCnt) 
		{
			stSetRtbl.mWeight = atoi(vParam[++j].c_str()); continue;
		}
		else if(vParam[j] == "-k" && j + 1 < iCnt) 
		{
			stSetRtbl.mTasks = atoi(vParam[++j].c_str()); continue;
		}
		else if(vParam[j] == "-t" && j + 1 < iCnt) 
		{
			stSetRtbl.mTimeline = atoi(vParam[++j].c_str()); continue;
		}
		else if(vParam[j] == "-c" && j + 1 < iCnt) 
		{
			stSetRtbl.mConnTime = atoi(vParam[++j].c_str()); continue;
		}
		else if(vParam[j] == "-s" && j + 1 < iCnt) 
		{
			stSetRtbl.mSuggest = atoi(vParam[++j].c_str()); continue;
		}
		else 
		{ 
			cout << "Unknow param." << endl;
			return -1;
		}
	}

	if(stSetRtbl.mId == 0)
	{
		cout << "need -i param" << endl;
	}
	else
	{
		SetWaitResStatus();
		return SyncSend((char *)&stSetRtbl, sizeof(stSetRtbl));
	}
	return -1;
}

//report -i 100 [-o 0.1] [-d 100]
int AgentCmd::ReportCmd(string sCmd, vector<string> vParam)
{
	SvrReportReqId_t stReportSvr;
	int iCnt = vParam.size();
	for(size_t j = 1; j < vParam.size(); j++)
	{
		if(vParam[j] == "-i" && j + 1 < iCnt) 
		{
			stReportSvr.mId = atoi(vParam[++j].c_str()); continue;
		}
		else if(vParam[j] == "-d" && j + 1 < iCnt) 
		{
			stReportSvr.mDelay = atoi(vParam[++j].c_str()); continue;
		}
		else if(vParam[j] == "-o" && j + 1 < iCnt) 
		{
			stReportSvr.mOkRate = atof(vParam[++j].c_str()); continue;
		}
		else 
		{ 
			cout << "Unknow param." << endl;
			return -1;
		}
	}

	if(stReportSvr.mId == 0)
	{
		cout << "need -i param" << endl;
	}
	else
	{
		SetWaitResStatus();
		mOutMsgQueue->Push((char *)&stReportSvr, sizeof(stReportSvr));	//保存输出共享内存
		cout << "report message success" << endl;
		
		SetWaitResStatus(false);
		LOG_INFO("default","[runtime] report a message(shm) success");
		return 0;
	}
	return -1;
}

//reload
int AgentCmd::ReloadCmd(string sCmd, vector<string> vParam)
{
	SetWaitResStatus();
	SvrReqReload_t vRtl;
	return SyncSend((char *)&vRtl, sizeof(vRtl));
}

//restart agent/router
int AgentCmd::RestartCmd(string sCmd, vector<string> vParam)
{
	SetWaitResStatus();
	return -1;
}
