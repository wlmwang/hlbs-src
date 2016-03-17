
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include "wAssert.h"
#include "RouterServerTask.h"
#include "RouterConfig.h"
#include "LoginCmd.h"

RouterServerTask::RouterServerTask()
{
    Initialize();
}

RouterServerTask::RouterServerTask(wIO *pIO) : wTcpTask(pIO)
{
    Initialize();
}

RouterServerTask::~RouterServerTask()
{
    //...
}

void RouterServerTask::Initialize()
{
	ROUTER_REG_DISP(CMD_SVR_REQ, SVR_REQ_INIT, &RouterServerTask::InitSvrReq);
	ROUTER_REG_DISP(CMD_SVR_REQ, SVR_REQ_RELOAD, &RouterServerTask::ReloadSvrReq);
	ROUTER_REG_DISP(CMD_SVR_REQ, SVR_REQ_SYNC, &RouterServerTask::SyncSvrReq);
}

//验证登录
int RouterServerTask::VerifyConn()
{
	if(!ROUTER_LOGIN) return 0;
	
	char pBuffer[ sizeof(LoginReqToken_t) ];
	int iLen = SyncRecv(pBuffer, sizeof(LoginReqToken_t));
	if (iLen > 0)
	{
		LoginReqToken_t *pLoginRes = (LoginReqToken_t*) pBuffer;
		if (strcmp(pLoginRes->mToken, "Anny") == 0)
		{
			LOG_ERROR("client", "[verify] receive client and verify success from ip(%s) port(%d) with token(%s)", mIO->Host().c_str(), mIO->Port(), pLoginRes->mToken);
			mConnType = pLoginRes->mConnType;
			return 0;
		}
		//LOG_ERROR("client", "[verify] receive client and verify failed from ip(%s) port(%d) with token(%s)", mIO->Host().c_str(), mIO->Port(), pLoginRes->mToken);
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

/**
 *  接受数据
 */
int RouterServerTask::HandleRecvMessage(char * pBuffer, int nLen)
{
	W_ASSERT(pBuffer != NULL, return -1);
	//解析消息
	struct wCommand *pCommand = (struct wCommand*) pBuffer;
	return ParseRecvMessage(pCommand, pBuffer, nLen);
}

int RouterServerTask::ParseRecvMessage(struct wCommand* pCommand, char *pBuffer, int iLen)
{
	if (pCommand->GetId() == CMD_ID(CMD_NULL, PARA_NULL))
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
			LOG_DEBUG("default", "[runtime] client fd(%d) send a invalid msg id(%u)", mIO->FD(), pCommand->GetId());
		}
	}
	return 0;
}

int RouterServerTask::InitSvrReq(char *pBuffer, int iLen)
{
	RouterConfig *pConfig = RouterConfig::Instance();

	SvrResInit_t vRRt;
	vRRt.mCode = 0;
	vRRt.mNum = pConfig->GetSvrAll(vRRt.mSvr);	//SvrNet_t
	SyncSend((char *)&vRRt, sizeof(vRRt));
	return 0;
}

int RouterServerTask::ReloadSvrReq(char *pBuffer, int iLen)
{
	RouterConfig *pConfig = RouterConfig::Instance();

	SvrResReload_t vRRt;
	vRRt.mCode = 0;
	vRRt.mNum = pConfig->ReloadSvr(vRRt.mSvr);	//SvrNet_t
	SyncSend((char *)&vRRt, sizeof(vRRt));
	return 0;
}

//agent请求下发修改的svr。一般不需要，由router主动下发
int RouterServerTask::SyncSvrReq(char *pBuffer, int iLen)
{
	RouterConfig *pConfig = RouterConfig::Instance();
	if (pConfig->IsModTime())
	{
		SvrResSync_t vRRt;
		vRRt.mCode = 0;
		vRRt.mNum = pConfig->GetModSvr(vRRt.mSvr);	//SvrNet_t
		if (vRRt.mNum > 0)
		{
			SyncSend((char *)&vRRt, sizeof(vRRt));
		}
	}
	return 0;
}
