
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "RouterServerTask.h"
#include "SvrCmd.h"
#include "LoginCmd.h"

RouterServerTask::RouterServerTask()
{
    Initialize();
}

RouterServerTask::RouterServerTask(wSocket *pSocket) : wTcpTask(pSocket)
{
    Initialize();
}

void RouterServerTask::Initialize()
{
    mConfig = RouterConfig::Instance();
    mServer = RouterServer::Instance();
	REG_DISP(mDispatch, "RouterServerTask", CMD_SVR_REQ, SVR_REQ_INIT, &RouterServerTask::InitSvrReq);
	REG_DISP(mDispatch, "RouterServerTask", CMD_SVR_REQ, SVR_REQ_RELOAD, &RouterServerTask::ReloadSvrReq);
}

//验证登录
int RouterServerTask::VerifyConn()
{
	if(!ROUTER_LOGIN) return 0;
	
	char pBuffer[sizeof(LoginReqToken_t)];
	int iLen = SyncRecv(pBuffer, sizeof(LoginReqToken_t));
	if (iLen > 0)
	{
		LoginReqToken_t *pLoginRes = (LoginReqToken_t*) pBuffer;
		if (strcmp(pLoginRes->mToken, "Anny") == 0)
		{
			LOG_INFO(ELOG_KEY, "[client] receive client and verify success from ip(%s) port(%d) with token(%s)", mIO->Host().c_str(), mIO->Port(), pLoginRes->mToken);
			mConnType = pLoginRes->mConnType;
			return 0;
		}
	}
	return -1;
}

//发送登录验证
int RouterServerTask::Verify()
{
	if(!AGENT_LOGIN) return 0;
	
	LoginReqToken_t stLoginReq;
	stLoginReq.mConnType = SERVER_ROUTER;
	memcpy(stLoginReq.mToken, "Anny", 4);
	SyncSend((char*)&stLoginReq, sizeof(stLoginReq));
	return 0;
}

int RouterServerTask::HandleRecvMessage(char *pBuffer, int nLen)
{
	W_ASSERT(pBuffer != NULL, return -1);
	struct wCommand *pCommand = (struct wCommand*) pBuffer;
	return ParseRecvMessage(pCommand, pBuffer, nLen);
}

int RouterServerTask::ParseRecvMessage(struct wCommand* pCommand, char *pBuffer, int iLen)
{
	if (pCommand->GetId() == W_CMD(CMD_NULL, PARA_NULL))
	{
		//空消息(心跳返回)
		mHeartbeatTimes = 0;
	}
	else
	{
		auto pF = mDispatch.GetFuncT("RouterServerTask", pCommand->GetId());

		if (pF != NULL)
		{
			pF->mFunc(pBuffer, iLen);
		}
		else
		{
			LOG_ERROR(ELOG_KEY, "[system] client send a invalid msg fd(%d) id(%u)", mIO->FD(), pCommand->GetId());
		}
	}
	return 0;
}

/** 向单个agent发送init回应 */
int RouterServerTask::InitSvrReq(char *pBuffer, int iLen)
{
	struct SvrResInit_t vRRt;
	vRRt.mCode = 0;
	vRRt.mNum = mConfig->Qos()->GetSvrAll(vRRt.mSvr);	
	mServer->Send(this, (char *)&vRRt, sizeof(vRRt));
	return 0;
}

/** 向单个agent发送reload响应 */
int RouterServerTask::ReloadSvrReq(char *pBuffer, int iLen)
{
	struct SvrResReload_t vRRt;
	vRRt.mCode = 0;
	
	mConfig->Qos()->CleanNode();
	vRRt.mNum = mConfig->GetModSvr(vRRt.mSvr);
	mServer->Send(this, (char *)&vRRt, sizeof(vRRt));
	return 0;
}
