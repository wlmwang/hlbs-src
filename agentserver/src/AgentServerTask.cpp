
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include "AgentServer.h" 
#include "AgentServerTask.h"
#include "AgentConfig.h"
#include "wAssert.h"
#include "LoginCommand.h"

AgentServerTask::AgentServerTask()
{
    Initialize();
}

AgentServerTask::AgentServerTask(wSocket *pSocket):wTcpTask(pSocket)
{
    Initialize();
}

AgentServerTask::~AgentServerTask()
{
    //...
}

void AgentServerTask::Initialize()
{
	AGENT_REG_DISP(CMD_SVR_REQ, SVR_REQ_RELOAD, &AgentServerTask::ReloadSvrReq);
	AGENT_REG_DISP(CMD_SVR_REQ, SVR_REQ_ALL, &AgentServerTask::GetSvrAll);
	AGENT_REG_DISP(CMD_SVR_REQ, SVR_REQ_ID, &AgentServerTask::GetSvrById);
	AGENT_REG_DISP(CMD_SVR_REQ, SVR_REQ_GID, &AgentServerTask::GetSvrByGid);
	AGENT_REG_DISP(CMD_SVR_REQ, SVR_REQ_NAME, &AgentServerTask::GetSvrByName);
	AGENT_REG_DISP(CMD_SVR_REQ, SVR_REQ_GXID, &AgentServerTask::GetSvrByGXid);

	AGENT_REG_DISP(CMD_SVR_RES, SVR_RES_INIT, &AgentServerTask::InitSvrRes);
	AGENT_REG_DISP(CMD_SVR_RES, SVR_RES_RELOAD, &AgentServerTask::ReloadSvrRes);

	AGENT_REG_DISP(CLI_SVR_REQ, SVR_SET_REQ_ID, &AgentServerTask::SetSvrAttr);
}

int AgentServerTask::VerifyConn()
{
	//验证登录消息
	char pBuffer[ sizeof(LoginReqToken_t) ];
	int iLen = SyncRecv(pBuffer, sizeof(LoginReqToken_t));
	if (iLen > 0)
	{
		LoginReqToken_t *pLoginRes = (LoginReqToken_t*) pBuffer;
		if (strcmp(pLoginRes->mToken, "Anny") == 0)
		{
			LOG_ERROR("client", "[verify] receive client and verify success from ip(%s) port(%d) with token(%s)", Socket()->IPAddr().c_str(), Socket()->Port(), pLoginRes->mToken);
			mConnType = pLoginRes->mConnType;
			return 0;
		}
		LOG_ERROR("client", "[verify] receive client and verify failed from ip(%s) port(%d) with token(%s)", Socket()->IPAddr().c_str(), Socket()->Port(), pLoginRes->mToken);
	}
	return -1;
}

int AgentServerTask::Verify()
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
int AgentServerTask::HandleRecvMessage(char * pBuffer, int nLen)
{
	W_ASSERT(pBuffer != NULL, return -1);
	//解析消息
	struct wCommand *pCommand = (struct wCommand*) pBuffer;
	return ParseRecvMessage(pCommand , pBuffer, nLen);
}

int AgentServerTask::ParseRecvMessage(struct wCommand* pCommand, char *pBuffer, int iLen)
{
	if (pCommand->GetId() == ASSERT_CMD(CMD_NULL, PARA_NULL))
	{
		//空消息(心跳返回)
		mHeartbeatTimes = 0;
	}
	else
	{
		auto pF = mDispatch.GetFuncT("AgentServerTask", pCommand->GetId());

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

//router发来相响应
int AgentServerTask::InitSvrRes(char *pBuffer, int iLen)
{
	AgentConfig *pConfig = AgentConfig::Instance();
	SvrResInit_t *pCmd = (struct SvrResInit_t* )pBuffer;

	if (pCmd->mNum > 0) pConfig->InitSvr(pCmd->mSvr, pCmd->mNum);
	return 0;
}

//router发来响应
int AgentServerTask::ReloadSvrRes(char *pBuffer, int iLen)
{
	AgentConfig *pConfig = AgentConfig::Instance();
	SvrResReload_t *pCmd = (struct SvrResReload_t* )pBuffer;

	if (pCmd->mNum > 0) pConfig->ReloadSvr(pCmd->mSvr, pCmd->mNum);
	return 0;
}

//agentcmd发来请求  。需同步reload router
int AgentServerTask::ReloadSvrReq(char *pBuffer, int iLen)
{
	AgentConfig *pConfig = AgentConfig::Instance();
	SvrResReload_t *pCmd = (struct SvrResReload_t* )pBuffer;
	AgentServer *pServer = AgentServer::Instance();
	
	SvrSetResData_t vRRt;
	vRRt.mCode = pServer->ReloadSvrReq();
	SyncSend((char *)&vRRt, sizeof(vRRt));
	return 0;
}

//agentcmd发来请求
int AgentServerTask::SetSvrAttr(char *pBuffer, int iLen)
{
	AgentConfig *pConfig = AgentConfig::Instance();
	SvrSetReqId_t *pCmd = (struct SvrSetReqId_t* )pBuffer;
	
	SvrSetResData_t vRRt;
	vRRt.mCode = pConfig->SetSvrAttr(pCmd->mId, pCmd->mDisabled, pCmd->mWeight, pCmd->mTimeline, pCmd->mConnTime, pCmd->mTasks,  pCmd->mSuggest);
	SyncSend((char *)&vRRt, sizeof(vRRt));
	return 0;
}

//agentcmd发来请求
int AgentServerTask::GetSvrAll(char *pBuffer, int iLen)
{
	AgentConfig *pConfig = AgentConfig::Instance();
	SvrReqAll_t *pCmd = (struct SvrReqAll_t* )pBuffer;

	SvrResData_t vRRt;
	vRRt.mReqId = pCmd->GetId();
	vRRt.mNum = pConfig->GetSvrAll(vRRt.mSvr, 0);
	SyncSend((char *)&vRRt, sizeof(vRRt));
	return 0;
}

//agentcmd发来请求
int AgentServerTask::GetSvrById(char *pBuffer, int iLen)
{
	AgentConfig *pConfig = AgentConfig::Instance();
	SvrReqId_t *pCmd = (struct SvrReqId_t* )pBuffer;

	SvrResData_t vRRt;
	vRRt.mReqId = pCmd->GetId();
	vRRt.mNum = pConfig->GetSvrById(vRRt.mSvr, pCmd->mId);
	SyncSend((char *)&vRRt, sizeof(vRRt));
	return 0;
}

//agentcmd发来请求
int AgentServerTask::GetSvrByGid(char *pBuffer, int iLen)
{
	AgentConfig *pConfig = AgentConfig::Instance();
	SvrReqGid_t *pCmd = (struct SvrReqGid_t* )pBuffer;
	
	SvrResData_t vRRt;
	vRRt.mReqId = pCmd->GetId();
	vRRt.mNum = pConfig->GetSvrByGid(vRRt.mSvr, pCmd->mGid, 1);
	SyncSend((char *)&vRRt, sizeof(vRRt));
	return 0;
}

//agentcmd发来请求
int AgentServerTask::GetSvrByName(char *pBuffer, int iLen)
{
	AgentConfig *pConfig = AgentConfig::Instance();
	SvrReqName_t *pCmd = (struct SvrReqName_t* )pBuffer;
	
	SvrResData_t vRRt;
	vRRt.mReqId = pCmd->GetId();
	vRRt.mNum = pConfig->GetSvrByName(vRRt.mSvr, pCmd->mName, 1);
	SyncSend((char *)&vRRt, sizeof(vRRt));
	return 0;
}

//agentcmd发来请求
int AgentServerTask::GetSvrByGXid(char *pBuffer, int iLen)
{
	AgentConfig *pConfig = AgentConfig::Instance();
	SvrReqGXid_t *pCmd = (struct SvrReqGXid_t* )pBuffer;
	
	SvrResData_t vRRt;
	vRRt.mReqId = pCmd->GetId();
	vRRt.mNum = pConfig->GetSvrByGXid(vRRt.mSvr, pCmd->mGid, pCmd->mXid, 1);
	SyncSend((char *)&vRRt, sizeof(vRRt));
	return 0;
}
