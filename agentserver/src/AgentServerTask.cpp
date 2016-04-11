
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "AgentServerTask.h"

AgentServerTask::AgentServerTask()
{
    Initialize();
}

AgentServerTask::AgentServerTask(wIO *pIO) : wTcpTask(pIO)
{
    Initialize();
}

AgentServerTask::~AgentServerTask()
{
    //...
}

void AgentServerTask::Initialize()
{
    mConfig = AgentConfig::Instance();
    mServer = AgentServer::Instance();

	AGENT_REG_DISP(CMD_SVR_REQ, SVR_REQ_RELOAD, &AgentServerTask::ReloadSvrReq);
	AGENT_REG_DISP(CMD_SVR_REQ, SVR_REQ_ALL, &AgentServerTask::GetSvrAll);
	AGENT_REG_DISP(CMD_SVR_REQ, SVR_REQ_GXID, &AgentServerTask::GetSvrByGXid);
	AGENT_REG_DISP(CMD_SVR_REQ, SVR_REQ_REPORT, &AgentServerTask::ReportSvr);
}

//验证登录消息
int AgentServerTask::VerifyConn()
{
	if(!AGENT_LOGIN) return 0;
	
	char pBuffer[sizeof(LoginReqToken_t)];
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

//发送登录验证
int AgentServerTask::Verify()
{
	if(!CLIENT_LOGIN) return 0;	//客户端验证
	
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
	if (pCommand->GetId() == CMD_ID(CMD_NULL, PARA_NULL))
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
			LOG_ERROR(ELOG_KEY, "[runtime] client fd(%d) send a invalid msg id(%u)", mIO->FD(), pCommand->GetId());
		}
	}
	return 0;
}

//agentcmd发来请求，(正常由router主动下发，而非请求下发)
int AgentServerTask::SyncSvrReq(char *pBuffer, int iLen)
{
	SvrReqSync_t vRRt;
	mServer->Send(this, (char *)&vRRt, sizeof(vRRt));
	return 0;
}

//agentcmd发来请求。需同步 reload router
int AgentServerTask::ReloadSvrReq(char *pBuffer, int iLen)
{
	AgentServer *pServer = AgentServer::Instance();
	
	//TODO.
	//SvrSetResData_t vRRt;
	//vRRt.mCode = pServer->ReloadSvrReq();
	//SyncSend((char *)&vRRt, sizeof(vRRt));
	return 0;
}

//agentcmd发来请求
int AgentServerTask::GetSvrAll(char *pBuffer, int iLen)
{
	AgentConfig *pConfig = AgentConfig::Instance();
	SvrReqAll_t *pCmd = (struct SvrReqAll_t* )pBuffer;

	SvrResData_t vRRt;
	vRRt.mReqId = pCmd->GetId();
	vRRt.mNum = pConfig->Qos()->GetSvrAll(vRRt.mSvr);
	mServer->Send(this, (char *)&vRRt, sizeof(vRRt));
	return 0;
}

//agentcmd发来查询请求(重点考虑此接口~)
int AgentServerTask::GetSvrByGXid(char *pBuffer, int iLen)
{
	AgentConfig *pConfig = AgentConfig::Instance();
	SvrReqGXid_t *pCmd = (struct SvrReqGXid_t* )pBuffer;
	
	SvrResData_t vRRt;
	vRRt.mReqId = pCmd->GetId();
	
	vRRt.mSvr[0].mGid = pCmd->mGid;
	vRRt.mSvr[0].mXid = pCmd->mXid;
	if(pConfig->Qos()->QueryNode(vRRt.mSvr[0]) >= 0)
	{
		vRRt.mNum = 1;
	}

	LOG_DEBUG(ELOG_KEY, "[runtime] send svr agent num(%d) gid(%d),xid(%d),host(%s),port(%d),weight(%d),ver(%d)", vRRt.mNum,
		vRRt.mSvr[0].mGid, vRRt.mSvr[0].mXid, vRRt.mSvr[0].mHost, vRRt.mSvr[0].mPort, vRRt.mSvr[0].mWeight, vRRt.mSvr[0].mVersion);
	
	mServer->Send(this, (char *)&vRRt, sizeof(vRRt));
	return 0;
}

//agentcmd发来上报请求(重点考虑此接口~)
int AgentServerTask::ReportSvr(char *pBuffer, int iLen)
{
	AgentConfig *pConfig = AgentConfig::Instance();
	SvrReqReport_t *pCmd = (struct SvrReqReport_t* )pBuffer;
	
	SvrResReport_t vRRt;

	if(pConfig->Qos()->CallerNode(pCmd->mCaller) >= 0)
	{
		vRRt.mCode = 1;
	}

	LOG_DEBUG(ELOG_KEY, "[runtime] send svr report %d", vRRt.mCode);
	
	mServer->Send(this, (char *)&vRRt, sizeof(vRRt));
	return 0;
}
