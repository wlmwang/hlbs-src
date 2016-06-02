
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "AgentUDSocketTask.h"

AgentUDSocketTask::AgentUDSocketTask()
{
    Initialize();
}

AgentUDSocketTask::AgentUDSocketTask(wIO *pIO) : wTcpTask(pIO)
{
    Initialize();
}

AgentUDSocketTask::~AgentUDSocketTask()
{
    //...
}

void AgentUDSocketTask::Initialize()
{
    mConfig = AgentConfig::Instance();
    mServer = AgentServer::Instance();

	AGENT_UNIX_DISP(CMD_SVR_REQ, SVR_REQ_GXID, &AgentUDSocketTask::GetSvrByGXid);
	AGENT_UNIX_DISP(CMD_SVR_REQ, SVR_REQ_REPORT, &AgentUDSocketTask::ReportSvr);
}

/**
 *  接受数据
 */
int AgentUDSocketTask::HandleRecvMessage(char * pBuffer, int nLen)
{
	W_ASSERT(pBuffer != NULL, return -1);
	//解析消息
	struct wCommand *pCommand = (struct wCommand*) pBuffer;
	return ParseRecvMessage(pCommand , pBuffer, nLen);
}

int AgentUDSocketTask::ParseRecvMessage(struct wCommand* pCommand, char *pBuffer, int iLen)
{
	if (pCommand->GetId() == CMD_ID(CMD_NULL, PARA_NULL))
	{
		//空消息(心跳返回)
		mHeartbeatTimes = 0;
	}
	else
	{
		auto pF = mDispatch.GetFuncT("AgentUDSocketTask", pCommand->GetId());

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

//客户端查询请求(重点考虑此接口~)
int AgentUDSocketTask::GetSvrByGXid(char *pBuffer, int iLen)
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

	LOG_DEBUG(ELOG_KEY, "[system] (unix sock) send svr agent num(%d) gid(%d),xid(%d),host(%s),port(%d),weight(%d),ver(%d)", vRRt.mNum,
		vRRt.mSvr[0].mGid, vRRt.mSvr[0].mXid, vRRt.mSvr[0].mHost, vRRt.mSvr[0].mPort, vRRt.mSvr[0].mWeight, vRRt.mSvr[0].mVersion);
	
	mServer->Send(this, (char *)&vRRt, sizeof(vRRt));
	return 0;
}

//客户端发来上报请求(重点考虑此接口~)
int AgentUDSocketTask::ReportSvr(char *pBuffer, int iLen)
{
	AgentConfig *pConfig = AgentConfig::Instance();
	SvrReqReport_t *pCmd = (struct SvrReqReport_t* )pBuffer;
	
	SvrResReport_t vRRt;

	if(pConfig->Qos()->CallerNode(pCmd->mCaller) >= 0)
	{
		vRRt.mCode = 1;
	}

	LOG_DEBUG(ELOG_KEY, "[system] (unix sock) send svr report %d", vRRt.mCode);
	
	mServer->Send(this, (char *)&vRRt, sizeof(vRRt));
	return 0;
}
