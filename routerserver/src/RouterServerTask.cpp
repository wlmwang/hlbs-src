
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
	mDispatch.Register("RouterServerTask", ASSERT_CMD(CMD_RTBL_REQ, RTBL_REQ_RELOAD), REG_FUNC(ASSERT_CMD(CMD_RTBL_REQ, RTBL_REQ_RELOAD), &RouterServerTask::ReloadRtblReq));
	mDispatch.Register("RouterServerTask", ASSERT_CMD(CMD_RTBL_REQ, RTBL_REQ_INIT), REG_FUNC(ASSERT_CMD(CMD_RTBL_REQ, RTBL_REQ_INIT), &RouterServerTask::InitRtblReq));
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
			return 0;
		}
	}
	return -1;
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

int RouterServerTask::ReloadRtblReq(char *pBuffer, int iLen)
{
	RouterConfig *pConfig = RouterConfig::Instance();
	//RtblReqReload_t *pCmd = (struct RtblReqReload_t* )pBuffer;

	RtblResReload_t vRRt;
	vRRt.mNum = pConfig->ReloadRtbl(vRRt.mRtbl, 0);
	SyncSend((char *)&vRRt, sizeof(vRRt));
	return 0;
}

int RouterServerTask::InitRtblReq(char *pBuffer, int iLen)
{
	RouterConfig *pConfig = RouterConfig::Instance();
	//RtblReqAll_t *pCmd = (struct RtblReqAll_t* )pBuffer;

	RtblResInit_t vRRt;
	vRRt.mNum = pConfig->GetRtblAll(vRRt.mRtbl, 0);
	SyncSend((char *)&vRRt, sizeof(vRRt));
	return 0;
}

/*
int RouterServerTask::GetRtblAll(char *pBuffer, int iLen)
{
	RouterConfig *pConfig = RouterConfig::Instance();
	//RtblReqAll_t *pCmd = (struct RtblReqAll_t* )pBuffer;

	RtblResData_t vRRt;
	vRRt.mNum = pConfig->GetRtblAll(vRRt.mRtbl, 0);
	SyncSend((char *)&vRRt, sizeof(vRRt));
	return 0;
}

int RouterServerTask::GetRtblById(char *pBuffer, int iLen)
{
	RouterConfig *pConfig = RouterConfig::Instance();
	RtblReqId_t *pCmd = (struct RtblReqId_t* )pBuffer;

	RtblResData_t vRRt;
	vRRt.mNum = pConfig->GetRtblById(vRRt.mRtbl, pCmd->mId);
	SyncSend((char *)&vRRt, sizeof(vRRt));
	return 0;
}

int RouterServerTask::GetRtblByGid(char *pBuffer, int iLen)
{
	RouterConfig *pConfig = RouterConfig::Instance();
	RtblReqGid_t *pCmd = (struct RtblReqGid_t* )pBuffer;
	
	RtblResData_t vRRt;
	vRRt.mNum = pConfig->GetRtblByGid(vRRt.mRtbl, pCmd->mGid, 1);
	SyncSend((char *)&vRRt, sizeof(vRRt));
	return 0;
}

int RouterServerTask::GetRtblByName(char *pBuffer, int iLen)
{
	RouterConfig *pConfig = RouterConfig::Instance();
	RtblReqName_t *pCmd = (struct RtblReqName_t* )pBuffer;
	
	RtblResData_t vRRt;
	vRRt.mNum = pConfig->GetRtblByName(vRRt.mRtbl, pCmd->mName, 1);
	SyncSend((char *)&vRRt, sizeof(vRRt));
	return 0;
}

int RouterServerTask::GetRtblByGXid(char *pBuffer, int iLen)
{
	RouterConfig *pConfig = RouterConfig::Instance();
	RtblReqGXid_t *pCmd = (struct RtblReqGXid_t* )pBuffer;
	
	RtblResData_t vRRt;
	vRRt.mNum = pConfig->GetRtblByGXid(vRRt.mRtbl, pCmd->mGid, pCmd->mXid, 1);
	SyncSend((char *)&vRRt, sizeof(vRRt));
	return 0;
}
*/