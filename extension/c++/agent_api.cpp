
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
	
	if(g_handle.mSock == NULL || g_handle.mTask == NULL)
	{
		ConnectAgent(&g_handle);
	}
	if (g_handle.mSock != NULL && g_handle.mTask != NULL)
	{
		//查询请求
		if(g_handle.mTask->SyncSend((char*)&stCmd, sizeof(stCmd)) > 0)
		{
			//接受返回
			struct SvrResData_t vRRt;
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
		}
		return -2;
	}
	return -1;
}

int NotifyCallerRes(const struct SvrNet_t &stSvr, int iResult, long long iUsetimeUsec, string &sErr)
{
	struct SvrReqReport_t stReportSvr;

	stReportSvr.mCaller.mCalledGid = stSvr.mGid;
	stReportSvr.mCaller.mCalledXid = stSvr.mXid;
	stReportSvr.mCaller.mPort = stSvr.mPort;
	stReportSvr.mCaller.mReqRet = iResult;
	stReportSvr.mCaller.mReqCount = 1;
	stReportSvr.mCaller.mReqUsetimeUsec = iUsetimeUsec;
	memcpy(stReportSvr.mCaller.mHost, stSvr.mHost, strlen(stSvr.mHost) + 1);

	if(g_handle.mShm == NULL || g_handle.mQueue == NULL)
	{
		InitShm(&g_handle);
	}
	if (g_handle.mShm != NULL && g_handle.mQueue != NULL)
	{
		return g_handle.mQueue->Push((char *)&stReportSvr, sizeof(struct SvrReqReport_t));
	}
	return -1;
}

int NotifyCallerNum(const struct SvrNet_t &stSvr, int iReqCount)
{
	return -1;
}

int InitShm(struct postHandle_t *pHandle)
{
	char *pAddr = 0;
	pHandle->mShm = new wShm(AGENT_SHM, 'i', MSG_QUEUE_LEN);
	if((pHandle->mShm->AttachShm() != NULL) && ((pAddr = pHandle->mShm->AllocShm(MSG_QUEUE_LEN)) != NULL))
	{
		pHandle->mQueue = new wMsgQueue();
		if (pHandle->mQueue != NULL)
		{
			pHandle->mQueue->SetBuffer(pAddr, MSG_QUEUE_LEN);
			return 0;
		}
		return -2;
	}
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
		SAFE_DELETE(pHandle->mShm);
		SAFE_DELETE(pHandle->mSock);
	}
}
