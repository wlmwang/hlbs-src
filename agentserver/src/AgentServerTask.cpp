
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
	AGENT_REG_DISP(CMD_RTBL_REQ, RTBL_REQ_RELOAD, &AgentServerTask::ReloadRtblReq);
	AGENT_REG_DISP(CMD_RTBL_REQ, RTBL_REQ_ALL, &AgentServerTask::GetRtblAll);
	AGENT_REG_DISP(CMD_RTBL_REQ, RTBL_REQ_ID, &AgentServerTask::GetRtblById);
	AGENT_REG_DISP(CMD_RTBL_REQ, RTBL_REQ_GID, &AgentServerTask::GetRtblByGid);
	AGENT_REG_DISP(CMD_RTBL_REQ, RTBL_REQ_NAME, &AgentServerTask::GetRtblByName);
	AGENT_REG_DISP(CMD_RTBL_REQ, RTBL_REQ_GXID, &AgentServerTask::GetRtblByGXid);
	AGENT_REG_DISP(CMD_RTBL_RES, RTBL_RES_INIT, &AgentServerTask::InitRtblRes);
	AGENT_REG_DISP(CMD_RTBL_RES, RTBL_RES_RELOAD, &AgentServerTask::ReloadRtblRes);
	AGENT_REG_DISP(CMD_RTBL_SET_REQ, RTBL_SET_REQ_ID, &AgentServerTask::SetRtblAttr);
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
			mConnType = pLoginRes->mConnType;
			return 0;
		}
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
int AgentServerTask::InitRtblRes(char *pBuffer, int iLen)
{
	AgentConfig *pConfig = AgentConfig::Instance();
	RtblResInit_t *pCmd = (struct RtblResInit_t* )pBuffer;

	if (pCmd->mNum > 0) pConfig->InitRtbl(pCmd->mRtbl, pCmd->mNum);
	return 0;
}

//router发来响应
int AgentServerTask::ReloadRtblRes(char *pBuffer, int iLen)
{
	AgentConfig *pConfig = AgentConfig::Instance();
	RtblResReload_t *pCmd = (struct RtblResReload_t* )pBuffer;

	if (pCmd->mNum > 0) pConfig->ReloadRtbl(pCmd->mRtbl, pCmd->mNum);
	return 0;
}

//agentcmd发来请求  。需同步reload router
int AgentServerTask::ReloadRtblReq(char *pBuffer, int iLen)
{
	AgentConfig *pConfig = AgentConfig::Instance();
	RtblResReload_t *pCmd = (struct RtblResReload_t* )pBuffer;
	AgentServer *pServer = AgentServer::Instance();
	
	RtblUpdateResData_t vRRt;
	vRRt.mRes = pServer->ReloadRtblReq();
	SyncSend((char *)&vRRt, sizeof(vRRt));
	return 0;
}

//agentcmd发来请求
int AgentServerTask::SetRtblAttr(char *pBuffer, int iLen)
{
	AgentConfig *pConfig = AgentConfig::Instance();
	RtblSetReqId_t *pCmd = (struct RtblSetReqId_t* )pBuffer;
	
	RtblUpdateResData_t vRRt;
	vRRt.mRes = pConfig->SetRtblAttr(pCmd->mId, pCmd->mDisabled, pCmd->mWeight, pCmd->mTimeline, pCmd->mConnTime, pCmd->mTasks,  pCmd->mSuggest);
	SyncSend((char *)&vRRt, sizeof(vRRt));
	return 0;
}

//agentcmd发来请求
int AgentServerTask::GetRtblAll(char *pBuffer, int iLen)
{
	AgentConfig *pConfig = AgentConfig::Instance();
	RtblReqAll_t *pCmd = (struct RtblReqAll_t* )pBuffer;

	RtblResData_t vRRt;
	vRRt.mNum = pConfig->GetRtblAll(vRRt.mRtbl, 0);
	SyncSend((char *)&vRRt, sizeof(vRRt));
	return 0;
}

//agentcmd发来请求
int AgentServerTask::GetRtblById(char *pBuffer, int iLen)
{
	AgentConfig *pConfig = AgentConfig::Instance();
	RtblReqId_t *pCmd = (struct RtblReqId_t* )pBuffer;

	RtblResData_t vRRt;
	vRRt.mNum = pConfig->GetRtblById(vRRt.mRtbl, pCmd->mId);
	SyncSend((char *)&vRRt, sizeof(vRRt));
	return 0;
}

//agentcmd发来请求
int AgentServerTask::GetRtblByGid(char *pBuffer, int iLen)
{
	AgentConfig *pConfig = AgentConfig::Instance();
	RtblReqGid_t *pCmd = (struct RtblReqGid_t* )pBuffer;
	
	RtblResData_t vRRt;
	vRRt.mNum = pConfig->GetRtblByGid(vRRt.mRtbl, pCmd->mGid, 1);
	SyncSend((char *)&vRRt, sizeof(vRRt));
	return 0;
}

//agentcmd发来请求
int AgentServerTask::GetRtblByName(char *pBuffer, int iLen)
{
	AgentConfig *pConfig = AgentConfig::Instance();
	RtblReqName_t *pCmd = (struct RtblReqName_t* )pBuffer;
	
	RtblResData_t vRRt;
	vRRt.mNum = pConfig->GetRtblByName(vRRt.mRtbl, pCmd->mName, 1);
	SyncSend((char *)&vRRt, sizeof(vRRt));
	return 0;
}

//agentcmd发来请求
int AgentServerTask::GetRtblByGXid(char *pBuffer, int iLen)
{
	AgentConfig *pConfig = AgentConfig::Instance();
	RtblReqGXid_t *pCmd = (struct RtblReqGXid_t* )pBuffer;
	
	RtblResData_t vRRt;
	vRRt.mNum = pConfig->GetRtblByGXid(vRRt.mRtbl, pCmd->mGid, pCmd->mXid, 1);
	SyncSend((char *)&vRRt, sizeof(vRRt));
	return 0;
}
