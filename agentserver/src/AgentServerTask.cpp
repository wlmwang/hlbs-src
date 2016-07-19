
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "AgentServerTask.h"
#include "SvrCmd.h"
#include "LoginCmd.h"

AgentServerTask::AgentServerTask()
{
    Initialize();
}

AgentServerTask::AgentServerTask(wSocket *pSocket) : wTcpTask(pSocket)
{
    Initialize();
}

void AgentServerTask::Initialize()
{
    mConfig = AgentConfig::Instance();
    mServer = AgentServer::Instance();
	REG_DISP(mDispatch, "AgentServerTask", CMD_SVR_REQ, SVR_REQ_RELOAD, &AgentServerTask::ReloadSvrReq);
	REG_DISP(mDispatch, "AgentServerTask", CMD_SVR_REQ, SVR_REQ_ALL, &AgentServerTask::GetSvrAll);
	REG_DISP(mDispatch, "AgentServerTask", CMD_SVR_REQ, SVR_REQ_GXID, &AgentServerTask::GetSvrByGXid);
	REG_DISP(mDispatch, "AgentServerTask", CMD_SVR_REQ, SVR_REQ_REPORT, &AgentServerTask::ReportSvr);
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
			LOG_ERROR("system", "[client] receive client and verify success from ip(%s) port(%d) with token(%s)", 
				mSocket->Host().c_str(), mSocket->Port(), pLoginRes->mToken);
			mConnType = pLoginRes->mConnType;
			return 0;
		}
		LOG_ERROR("system", "[client] receive client and verify failed from ip(%s) port(%d) with token(%s)", 
			mSocket->Host().c_str(), mSocket->Port(), pLoginRes->mToken);
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
int AgentServerTask::HandleRecvMessage(char *pBuffer, int nLen)
{
	W_ASSERT(pBuffer != NULL, return -1);
	//解析消息
	struct wCommand *pCommand = (struct wCommand*) pBuffer;
	return ParseRecvMessage(pCommand , pBuffer, nLen);
}

int AgentServerTask::ParseRecvMessage(struct wCommand *pCommand, char *pBuffer, int iLen)
{
	if (pCommand->GetId() == W_CMD(CMD_NULL, PARA_NULL))
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
			LOG_ERROR(ELOG_KEY, "[system] client send a invalid msg fd(%d) id(%d)", mIO->FD(), pCommand->GetId());
		}
	}
	return 0;
}

//agentcmd发来请求，(正常由router主动下发，而非请求下发)
int AgentServerTask::SyncSvrReq(char *pBuffer, int iLen)
{
	struct SvrReqSync_t vRRt;
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
	struct SvrReqAll_t *pCmd = (struct SvrReqAll_t* )pBuffer;

	struct SvrResData_t vRRt;
	vRRt.mReqId = pCmd->GetId();
	vRRt.mNum = pConfig->Qos()->GetSvrAll(vRRt.mSvr);
	mServer->Send(this, (char *)&vRRt, sizeof(vRRt));
	return 0;
}

//客户端查询请求(重点考虑此接口~)
int AgentServerTask::GetSvrByGXid(char *pBuffer, int iLen)
{
	AgentConfig *pConfig = AgentConfig::Instance();
	struct SvrReqGXid_t *pCmd = (struct SvrReqGXid_t* )pBuffer;
	
	struct SvrOneRes_t vRRt;
	vRRt.mSvr.mGid = pCmd->mGid;
	vRRt.mSvr.mXid = pCmd->mXid;
	
	//手动测试该函数是否为瓶颈函数
	if (pConfig->Qos()->QueryNode(vRRt.mSvr) >= 0)
	{
		vRRt.mNum = 1;
	}
	mServer->Send(this, (char *)&vRRt, sizeof(vRRt));
	
	LOG_DEBUG(ELOG_KEY, "[system] send svr agent gid(%d),xid(%d),host(%s),port(%d),weight(%d),ver(%d)",
		vRRt.mSvr.mGid, vRRt.mSvr.mXid, vRRt.mSvr.mHost, vRRt.mSvr.mPort, vRRt.mSvr.mWeight, vRRt.mSvr.mVersion);

	return 0;
}

//客户端发来上报请求(重点考虑此接口~)
int AgentServerTask::ReportSvr(char *pBuffer, int iLen)
{
	AgentConfig *pConfig = AgentConfig::Instance();
	struct SvrReqReport_t *pCmd = (struct SvrReqReport_t* )pBuffer;
	
	struct SvrResReport_t vRRt;
	if (pConfig->Qos()->CallerNode(pCmd->mCaller) >= 0)
	{
		vRRt.mCode = 1;
	}
	mServer->Send(this, (char *)&vRRt, sizeof(vRRt));
	
	LOG_DEBUG(ELOG_KEY, "[system] send svr report ret %d, gid(%d) xid(%d),host(%s),port(%d)", 
		vRRt.mCode, pCmd->mCaller.mCalledGid, pCmd->mCaller.mCalledXid, pCmd->mCaller.mHost, pCmd->mCaller.mPort);

	return 0;
}
