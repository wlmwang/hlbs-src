
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include "wAssert.h"
#include "RouterServerTask.h"
#include "RouterConfig.h"
#include "LoginCommand.h"

RouterServerTask::RouterServerTask()
{
    Initialize();
}

RouterServerTask::RouterServerTask(wSocket *pSocket) : wTcpTask(pSocket)
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

int RouterServerTask::VerifyConn()
{
	//验证登录消息
	char pBuffer[ sizeof(LoginReqToken_t) ];
	int iLen = SyncRecv(pBuffer, sizeof(LoginReqToken_t));
	if (iLen > 0)
	{
		LoginReqToken_t *pLoginRes = (LoginReqToken_t*) pBuffer;
		if (strcmp(pLoginRes->mToken, "Anny") == 0)
		{
			mConnType = pLoginRes->mConnType;
			return 0;
		}
	}
	return -1;
}

int RouterServerTask::Verify()
{
	//验证登录
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
	if (pCommand->GetId() == ASSERT_CMD(CMD_NULL, PARA_NULL))
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
			LOG_DEBUG("default", "client fd(%d) send a invalid msg id(%u)", mSocket->SocketFD(), pCommand->GetId());
		}
	}
	return 0;
}

int RouterServerTask::InitSvrReq(char *pBuffer, int iLen)
{
	RouterConfig *pConfig = RouterConfig::Instance();

	SvrResInit_t vRRt;
	vRRt.mCode = 0;
	vRRt.mNum = pConfig->GetSvrAll(vRRt.mSvr, 0);
	SyncSend((char *)&vRRt, sizeof(vRRt));
	return 0;
}

int RouterServerTask::ReloadSvrReq(char *pBuffer, int iLen)
{
	RouterConfig *pConfig = RouterConfig::Instance();

	SvrResReload_t vRRt;
	vRRt.mCode = 0;
	vRRt.mNum = pConfig->ReloadSvr(vRRt.mSvr, 0);
	SyncSend((char *)&vRRt, sizeof(vRRt));
	return 0;
}

int RouterServerTask::SyncSvrReq(char *pBuffer, int iLen)
{
	RouterConfig *pConfig = RouterConfig::Instance();

	SvrResSync_t vRRt;
	vRRt.mCode = 0;
	vRRt.mNum = pConfig->SyncSvr(vRRt.mSvr, 0);
	SyncSend((char *)&vRRt, sizeof(vRRt));
	return 0;
}
