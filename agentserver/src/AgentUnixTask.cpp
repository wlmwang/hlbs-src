
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "SvrCmd.h"
#include "LoginCmd.h"
#include "AgentUnixTask.h"

AgentUnixTask::AgentUnixTask()
{
    Initialize();
}

AgentUnixTask::AgentUnixTask(wSocket *pSocket) : wUnixTask(pSocket)
{
    Initialize();
}

void AgentUnixTask::Initialize()
{
    mConfig = AgentConfig::Instance();
    mServer = AgentServer::Instance();
	REG_DISP(mDispatch, "AgentUnixTask", CMD_SVR_REQ, SVR_REQ_GXID, &AgentUnixTask::GetSvrByGXid);
	REG_DISP(mDispatch, "AgentUnixTask", CMD_SVR_REQ, SVR_REQ_REPORT, &AgentUnixTask::ReportSvr);
}

/**
 *  接受数据
 */
int AgentUnixTask::HandleRecvMessage(char *pBuffer, int nLen)
{
	W_ASSERT(pBuffer != NULL, return -1);
	//解析消息
	struct wCommand *pCommand = (struct wCommand*) pBuffer;
	return ParseRecvMessage(pCommand , pBuffer, nLen);
}

int AgentUnixTask::ParseRecvMessage(struct wCommand *pCommand, char *pBuffer, int iLen)
{
	if (pCommand->GetId() == W_CMD(CMD_NULL, PARA_NULL))
	{
		//空消息(心跳返回)
		mHeartbeatTimes = 0;
	}
	else
	{
		auto pF = mDispatch.GetFuncT("AgentUnixTask", pCommand->GetId());

		if (pF != NULL)
		{
			pF->mFunc(pBuffer, iLen);
		}
		else
		{
			LOG_ERROR(ELOG_KEY, "[system] unix client send a invalid msg fd(%d) id(%d)", mSocket->FD(), pCommand->GetId());
		}
	}
	return 0;
}

//客户端查询请求(重点考虑此接口~)
int AgentUnixTask::GetSvrByGXid(char *pBuffer, int iLen)
{
	AgentConfig *pConfig = AgentConfig::Instance();
	struct SvrReqGXid_t *pCmd = (struct SvrReqGXid_t* )pBuffer;
	
	struct SvrOneRes_t vRRt;
	vRRt.mSvr.mGid = pCmd->mGid;
	vRRt.mSvr.mXid = pCmd->mXid;
	if (pConfig->Qos()->QueryNode(vRRt.mSvr) >= 0)
	{
		mServer->Send(this, (char *)&vRRt, sizeof(vRRt));
	}
	
	LOG_DEBUG(ELOG_KEY, "[system] (unix sock) send svr agent gid(%d),xid(%d),host(%s,%d),port(%d),weight(%d),ver(%d)",
		vRRt.mSvr.mGid, vRRt.mSvr.mXid, vRRt.mSvr.mHost, vRRt.mSvr.mPort, vRRt.mSvr.mWeight, vRRt.mSvr.mVersion);

	mServer->Send(this, (char *)&vRRt, sizeof(vRRt));
	return 0;
}

//客户端发来上报请求(重点考虑此接口~)
int AgentUnixTask::ReportSvr(char *pBuffer, int iLen)
{
	AgentConfig *pConfig = AgentConfig::Instance();
	struct SvrReqReport_t *pCmd = (struct SvrReqReport_t* )pBuffer;
	
	struct SvrResReport_t vRRt;
	if(pConfig->Qos()->CallerNode(pCmd->mCaller) >= 0)
	{
		vRRt.mCode = 1;
	}

	LOG_DEBUG(ELOG_KEY, "[system] (unix sock) send svr report %d", vRRt.mCode);
	
	mServer->Send(this, (char *)&vRRt, sizeof(vRRt));
	return 0;
}
