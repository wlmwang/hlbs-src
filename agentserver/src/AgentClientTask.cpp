
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include "AgentServer.h" 
#include "AgentConfig.h"
#include "LoginCmd.h"
#include "AgentClientTask.h"

AgentClientTask::AgentClientTask()
{
    Initialize();
}

AgentClientTask::AgentClientTask(wIO *pIO) : wTcpTask(pIO)
{
    Initialize();
}

AgentClientTask::~AgentClientTask()
{
    //...
}

void AgentClientTask::Initialize()
{
	AGENTCLT_REG_DISP(CMD_SVR_REQ, SVR_REQ_RELOAD, &AgentClientTask::ReloadSvrReq);
	AGENTCLT_REG_DISP(CMD_SVR_REQ, SVR_REQ_ALL, &AgentClientTask::GetSvrAll);
	AGENTCLT_REG_DISP(CMD_SVR_REQ, SVR_REQ_GXID, &AgentClientTask::GetSvrByGXid);

	AGENTCLT_REG_DISP(CMD_SVR_RES, SVR_RES_INIT, &AgentClientTask::InitSvrRes);
	AGENTCLT_REG_DISP(CMD_SVR_RES, SVR_RES_RELOAD, &AgentClientTask::ReloadSvrRes);
	AGENTCLT_REG_DISP(CMD_SVR_RES, SVR_RES_SYNC, &AgentClientTask::SyncSvrRes);
}

int AgentClientTask::VerifyConn()
{
	//验证登录消息
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
		LOG_ERROR("client", "[verify] receive client and verify failed from ip(%s) port(%d) with token(%s)", mIO->Host().c_str(), mIO->Port(), pLoginRes->mToken);
	}
	return -1;
}

int AgentClientTask::Verify()
{
	//验证登录
	LoginReqToken_t stLoginReq;
	stLoginReq.mConnType = SERVER_AGENT;
	memcpy(stLoginReq.mToken, "Anny", 4);
	SyncSend((char*)&stLoginReq, sizeof(stLoginReq));
	return 0;
}

/**
 *  接受数据
 */
int AgentClientTask::HandleRecvMessage(char * pBuffer, int nLen)
{
	W_ASSERT(pBuffer != NULL, return -1);
	//解析消息
	struct wCommand *pCommand = (struct wCommand*) pBuffer;
	return ParseRecvMessage(pCommand , pBuffer, nLen);
}

int AgentClientTask::ParseRecvMessage(struct wCommand* pCommand, char *pBuffer, int iLen)
{
	if (pCommand->GetId() == CMD_ID(CMD_NULL, PARA_NULL))
	{
		//空消息(心跳返回)
		mHeartbeatTimes = 0;
	}
	else
	{
		auto pF = mDispatch.GetFuncT("AgentClientTask", pCommand->GetId());

		if (pF != NULL)
		{
			pF->mFunc(pBuffer, iLen);
		}
		else
		{
			LOG_ERROR(ELOG_KEY, "[runtime] client fd(%d) send a invalid msg id(%u)", mIO->FD(), pCommand->GetId());
		}
	}
	return 0;
}

//router发来init相响应
int AgentClientTask::InitSvrRes(char *pBuffer, int iLen)
{
	AgentConfig *pConfig = AgentConfig::Instance();
	SvrResInit_t *pCmd = (struct SvrResInit_t* )pBuffer;

	if (pCmd->mNum > 0) pConfig->InitSvr(pCmd->mSvr, pCmd->mNum);
	return 0;
}

//router发来reload响应
int AgentClientTask::ReloadSvrRes(char *pBuffer, int iLen)
{
	AgentConfig *pConfig = AgentConfig::Instance();
	SvrResReload_t *pCmd = (struct SvrResReload_t* )pBuffer;

	if (pCmd->mNum > 0) pConfig->ReloadSvr(pCmd->mSvr, pCmd->mNum);
	return 0;
}

//router发来sync响应
int AgentClientTask::SyncSvrRes(char *pBuffer, int iLen)
{
	AgentConfig *pConfig = AgentConfig::Instance();
	SvrResSync_t *pCmd = (struct SvrResSync_t* )pBuffer;

	if (pCmd->mNum > 0) pConfig->SyncSvr(pCmd->mSvr, pCmd->mNum);
	return 0;
}

//agentcmd发来请求，(正常由router主动下发，而非请求下发)
int AgentClientTask::SyncSvrReq(char *pBuffer, int iLen)
{
	SvrReqSync_t vRRt;
	SyncSend((char *)&vRRt, sizeof(vRRt));
	return 0;
}

//agentcmd发来请求  。需同步reload router
int AgentClientTask::ReloadSvrReq(char *pBuffer, int iLen)
{
	AgentServer *pServer = AgentServer::Instance();
	
	SvrSetResData_t vRRt;
	vRRt.mCode = pServer->ReloadSvrReq();
	SyncSend((char *)&vRRt, sizeof(vRRt));
	return 0;
}

//agentcmd发来请求
int AgentClientTask::GetSvrAll(char *pBuffer, int iLen)
{
	AgentConfig *pConfig = AgentConfig::Instance();
	SvrReqAll_t *pCmd = (struct SvrReqAll_t* )pBuffer;

	SvrResData_t vRRt;
	vRRt.mReqId = pCmd->GetId();
	vRRt.mNum = pConfig->GetSvrAll(vRRt.mSvr);
	SyncSend((char *)&vRRt, sizeof(vRRt));
	return 0;
}

//agentcmd发来请求(重点考虑此接口~)
int AgentClientTask::GetSvrByGXid(char *pBuffer, int iLen)
{
	AgentConfig *pConfig = AgentConfig::Instance();
	SvrReqGXid_t *pCmd = (struct SvrReqGXid_t* )pBuffer;
	
	SvrResData_t vRRt;
	vRRt.mReqId = pCmd->GetId();
	vRRt.mNum = 1;
	pConfig->GetSvrByGXid(vRRt.mSvr, pCmd->mGid, pCmd->mXid);
	SyncSend((char *)&vRRt, sizeof(vRRt));
	return 0;
}
