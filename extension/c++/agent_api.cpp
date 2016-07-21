
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

	if (ConnectAgent() < 0) return iRet;

	//查询请求
	if (g_handle.mTask != NULL && g_handle.mTask->SyncSend((char*)&stCmd, sizeof(stCmd)) > 0)
	{
		//接受返回
		char vBuffer[sizeof(struct SvrOneRes_t)];
		int iLen = g_handle.mTask->SyncRecv(vBuffer, sizeof(struct SvrOneRes_t), iTimeOut);
		if (iLen > 0)
		{
			struct SvrOneRes_t *pRes = (struct SvrOneRes_t*) vBuffer;
			stSvr.mPort = pRes->mSvr.mPort;
			memcpy(stSvr.mHost, pRes->mSvr.mHost, strlen(pRes->mSvr.mHost) + 1);
			iRet = 0;
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
	
	CloseAgent();
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

	if (ConnectAgent() < 0) return iRet;

	//上报请求
	if (g_handle.mTask != NULL && g_handle.mTask->SyncSend((char*)&stCmd, sizeof(stCmd)) > 0)
	{
		//接受返回
		char vBuffer[sizeof(struct SvrResReport_t)];
		int iLen = g_handle.mTask->SyncRecv(vBuffer, sizeof(struct SvrResReport_t));
		if (iLen > 0)
		{
			struct SvrResReport_t *pRes = (struct SvrResReport_t*) vBuffer;
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

	CloseAgent();
	return iRet;
}

int NotifyCallerNum(const struct SvrNet_t &stSvr, int iReqCount)
{
	return -1;
}

int ConnectAgent()
{
	if (g_handle.mConnecting == true && g_handle.mTask != NULL) return 0;
	
	CloseAgent();

#if !SOCKET_HANDLE
	wSocket *pSocket = new wTcpSocket(SOCK_TYPE_CONNECT);
#else
	wSocket *pSocket = new wUnixSocket(SOCK_TYPE_CONNECT);
#endif
	
	if (pSocket->Open() >= 0)
	{
		if (pSocket->SockProto() == SOCK_PROTO_TCP)
		{
			if (pSocket->Connect(AGENT_HOST, AGENT_PORT) >= 0)
			{
				g_handle.mTask = new wTask(pSocket);
				return 0;
			}
		}
		else if (pSocket->SockProto() == SOCK_PROTO_UNIX)
		{
			if (pSocket->Connect(AGENT_HOST, AGENT_TIMEOUT) >= 0)
			{
				g_handle.mTask = new wTask(pSocket);
				return 0;
			}
		}
	}

	CloseAgent();
	return -1;
}

void CloseAgent()
{
	g_handle.mConnecting = false;
	SAFE_DELETE(g_handle.mTask);
}
