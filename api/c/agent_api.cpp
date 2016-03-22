
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include "agent_api.h"

//post_handle_t g_handle;

int NotifyCallerRes(int iGid, int iXid, const char* vHost, int iPort, int iReqRet, int iReqCount)
{
	//上报调用结果
	struct SvrReqReport_t stReportSvr;
	stReportSvr.mCaller.mCalledGid = iGid;
	stReportSvr.mCaller.mCalledXid = iXid;
	stReportSvr.mCaller.mPort = iPort;
	memcpy(stReportSvr.mCaller.mHost, vHost, strlen(vHost));

	stReportSvr.mCaller.mReqRet = iReqRet;
	stReportSvr.mCaller.mReqCount = iReqCount;
	
	return NotifyAgentSvr(stReportSvr);
}

int NotifyCaller(int iGid, int iXid, const char* vHost, int iPort, int iReqCount)
{
	return 0;
}

int QueryNode(int iGid, int iXid, char* vHost, int* pPort, int *pWeight)
{
	return 0;
}

int NotifyAgentSvr(SvrReqReport_t *pReport)
{
	struct post_handle_t handle;
	if (HlfsStart(&handle) == -1)
	{
		return -1;
	}
	handle.queue->Push((char *)&pReport, sizeof(pReport));
}

int HlfsStart(struct post_handle_t *handle)
{
	char *pAddr = 0;
	handle->shm = new wShm(IPC_SHM, 'i', MSG_QUEUE_LEN);
	if((handle->shm->AttachShm() != NULL) && ((pAddr = handle->shm->AllocShm(MSG_QUEUE_LEN)) != NULL))
	{
		handle->queue = new wMsgQueue();
		handle->queue->SetBuffer(pAddr, MSG_QUEUE_LEN);
		return 0;
	}
	return -1;
}

void HlfsFinal(struct post_handle_t *handle)
{
	if(handle != NULL)
	{
		SAFE_DELETE(handle->shm);
		SAFE_DELETE(handle->queue);
	}
}
