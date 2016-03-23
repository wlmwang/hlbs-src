
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include "agent_api.h"

struct postHandle_t g_handle;

int QueryNode(struct SvrNet_t &stSvr, double iTimeOut, string &sErr)
{
	if (ConnectAgent(&g_handle) < 0)
	{
		return -1;
	}
	if (g_handle.mSvrQos)
	{
		g_handle.mSvrQos->QueryNode(stSvr);
		return 0;
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

	if (InitShm(&g_handle) < 0)
	{
		return -1;
	}
	if (g_handle.mQueue)
	{
		return g_handle.mQueue->Push((char *)&stReportSvr, sizeof(struct SvrReqReport_t));
	}
	return -1;
}

int NotifyCallerNum(const struct SvrNet_t &stSvr, int iReqCount)
{
	return 0;
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
	if(pHandle->mSock->Open() > 0)
	{
		if(pHandle->mSock->Connect(AGENT_HOST, AGENT_PORT) > 0)
		{
			pHandle->mSvrQos = SvrQos::Instance();
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
		SAFE_DELETE(pHandle->mQueue);
		SAFE_DELETE(pHandle->mSock);
	}
}
