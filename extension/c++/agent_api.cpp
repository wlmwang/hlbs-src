
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include "agent_api.h"

struct postHandle_t g_handle;

int QueryNode(struct SvrNet_t &stSvr, double iTimeOut, string &sErr)
{
	struct SvrReqGXid_t stCmd;
	stCmd.mGid = stSvr.mGid;
	stCmd.mXid = stSvr.mXid;
	
	//if (g_handle.mSock == NULL || g_handle.mTask == NULL)
	{
		if(ConnectAgent(&g_handle) < 0)
		{
			return -1;
		}
	}
	if (g_handle.mSock != NULL && g_handle.mTask != NULL)
	{
		//查询请求
		if(g_handle.mTask->SyncSend((char*)&stCmd, sizeof(stCmd)) > 0)
		{
			//接受返回
			char pBuffer[sizeof(struct SvrResData_t)];
			int iLen = g_handle.mTask->SyncRecv(pBuffer, sizeof(struct SvrResData_t));
			if (iLen > 0)
			{
				SvrResData_t *pRes = (SvrResData_t*) pBuffer;
				if(pRes->mNum == 1)
				{
					stSvr.mPort = pRes->mSvr[0].mPort;
					memcpy(stSvr.mHost, pRes->mSvr[0].mHost, strlen(pRes->mSvr[0].mHost) + 1);
					return 0;
				}
			}
			return -3;
		}
		return -2;
	}
	return -1;
}

int NotifyCallerRes(const struct SvrNet_t &stSvr, int iResult, long long iUsetimeUsec, string &sErr)
{
	struct SvrReqReport_t stCmd;

	stCmd.mCaller.mCalledGid = stSvr.mGid;
	stCmd.mCaller.mCalledXid = stSvr.mXid;
	stCmd.mCaller.mPort = stSvr.mPort;
	stCmd.mCaller.mReqRet = iResult;
	stCmd.mCaller.mReqCount = 1;
	stCmd.mCaller.mReqUsetimeUsec = iUsetimeUsec;
	memcpy(stCmd.mCaller.mHost, stSvr.mHost, strlen(stSvr.mHost) + 1);

	//if (g_handle.mSock == NULL || g_handle.mTask == NULL)
	{
		if(ConnectAgent(&g_handle) < 0)
		{
			return -1;
		}
	}

	if (g_handle.mSock != NULL && g_handle.mTask != NULL)
	{
		//上报请求
		if(g_handle.mTask->SyncSend((char*)&stCmd, sizeof(stCmd)) > 0)
		{
			//接受返回
			char pBuffer[sizeof(struct SvrResReport_t)];
			int iLen = g_handle.mTask->SyncRecv(pBuffer, sizeof(struct SvrResReport_t));
			if (iLen > 0)
			{
				struct SvrResReport_t *pRes = (struct SvrResReport_t*) pBuffer;
				return pRes->mCode;
			}
		}
		return -2;
	}
	return -1;
}

int NotifyCallerNum(const struct SvrNet_t &stSvr, int iReqCount)
{
	return -1;
}

int ConnectAgent(struct postHandle_t *pHandle)
{
	pHandle->mSock = new wSocket();
	if(pHandle->mSock->Open() >= 0)
	{
		if(pHandle->mSock->Connect(AGENT_HOST, AGENT_PORT) >= 0)
		{
			pHandle->mTask = new wTask(pHandle->mSock);
			return 0;
		}
		return -2;
	}
	return -1;
}

void Release(struct postHandle_t *pHandle)
{
	if(pHandle != NULL)
	{
		SAFE_DELETE(pHandle->mSock);
	}
}
