
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "agent_api.h"

struct postHandle_t g_handle;

int QueryNode(struct SvrNet_t &stSvr, double iTimeOut, string &sErr)
{
	struct SvrReqGXid_t stCmd;
	stCmd.mGid = stSvr.mGid;
	stCmd.mXid = stSvr.mXid;
	int iRet = -1;

	//if (ConnectAgent() < 0) return iRet;

	//查询请求
	if (g_handle.mTask.SyncSend((char*)&stCmd, sizeof(stCmd)) > 0)
	{
		//接受返回
		char pBuffer[sizeof(struct SvrResData_t)];
		int iLen = g_handle.mTask.SyncRecv(pBuffer, sizeof(struct SvrResData_t), iTimeOut);
		if (iLen > 0)
		{
			SvrResData_t *pRes = (SvrResData_t*) pBuffer;
			if (pRes->mNum == 1)
			{
				stSvr.mPort = pRes->mSvr[0].mPort;
				memcpy(stSvr.mHost, pRes->mSvr[0].mHost, strlen(pRes->mSvr[0].mHost) + 1);
				iRet = 0;
			}
		}
		else
		{
			iRet = -3;
		}
	}
	else
	{
		iRet = -2;
	}
	
	//CloseAgent();
	return iRet;
}

int NotifyCallerRes(const struct SvrNet_t &stSvr, int iResult, long long iUsetimeUsec, string &sErr)
{
	struct SvrReqReport_t stCmd;
	int iRet = -1;
	
	stCmd.mCaller.mCalledGid = stSvr.mGid;
	stCmd.mCaller.mCalledXid = stSvr.mXid;
	stCmd.mCaller.mPort = stSvr.mPort;
	stCmd.mCaller.mReqRet = iResult;
	stCmd.mCaller.mReqCount = 1;
	stCmd.mCaller.mReqUsetimeUsec = iUsetimeUsec;
	memcpy(stCmd.mCaller.mHost, stSvr.mHost, strlen(stSvr.mHost) + 1);

	//if (ConnectAgent() < 0) return iRet;

	//上报请求
	if (g_handle.mTask.SyncSend((char*)&stCmd, sizeof(stCmd)) > 0)
	{
		//接受返回
		char pBuffer[sizeof(struct SvrResReport_t)];
		int iLen = g_handle.mTask.SyncRecv(pBuffer, sizeof(struct SvrResReport_t));
		if (iLen > 0)
		{
			struct SvrResReport_t *pRes = (struct SvrResReport_t*) pBuffer;
			iRet = pRes->mCode;
		}
		else
		{
			iRet = -3;
		}
	}
	else
	{
		iRet = -2;
	}

	//CloseAgent();
	return iRet;
}

int NotifyCallerNum(const struct SvrNet_t &stSvr, int iReqCount)
{
	return -1;
}

int ConnectAgent()
{
	if (g_handle.mConnecting == true)
	{
		return 0;
	}

	int iRet = 0;
	if(g_handle.mSock.Open() >= 0)
	{
		if (g_handle.mSock.TaskType() == TASK_TCP)
		{
			if(g_handle.mSock.Connect(AGENT_HOST, AGENT_PORT) >= 0)
			{
				g_handle.mTask.mIO = &g_handle.mSock;
				g_handle.mConnecting = true;
				return 0;
			}
		}
		else if (g_handle.mSock.TaskType() == TASK_UNIXD)
		{
			if(g_handle.mSock.Connect(AGENT_HOST) >= 0)
			{
				g_handle.mTask.mIO = &g_handle.mSock;
				g_handle.mConnecting = true;
				return 0;
			}
		}
		iRet = -2;
	}
	else
	{
		iRet = -1;
	}

	CloseAgent();
	return iRet;
}

void CloseAgent()
{
	if (g_handle.mConnecting == true)
	{
		g_handle.mSock.Close();
		g_handle.mConnecting = false;
	}
}
