
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */
 
#include "AgentServerTask.h"
#include "AgentConfig.h"

#include "wAssert.h"

AgentServerTask::AgentServerTask(wSocket *pSocket):wTcpTask(pSocket)
{
    //...
}

AgentServerTask::~AgentServerTask()
{
    //...
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
	AgentConfig *pConfig = AgentConfig::Instance();
	switch(pCommand->GetId())
	{
		case ASSERT_CMD(CMD_NULL, PARA_NULL):
		{
			//空消息(心跳返回)
			mHeartbeatTimes = 0;
			break;
		}
		case ASSERT_CMD(CMD_RTBL_RES, RTBL_RES_DATA):
		{
			RtblResData_t *pCmd = (struct RtblResData_t* )pBuffer;

			if (pCmd->mNum > 0) pConfig->FixRtbl(pCmd->mRtbl, pCmd->mNum);
		}
		//
		case ASSERT_CMD(CMD_RTBL_REQ, RTBL_REQ_ALL):	//获取所有rtbl
		{
			RtblReqAll_t *pCmd = (struct RtblReqAll_t* )pBuffer;

			RtblResData_t vRRt;
			vRRt.mNum = pConfig->GetRtblAll(vRRt.mRtbl, 0);
			if(vRRt.mNum > 0) SyncSend((char *)&vRRt, sizeof(vRRt));
			break;
		}
		case ASSERT_CMD(CMD_RTBL_REQ, RTBL_REQ_ID):	//id方式获取rtbl
		{
			RtblReqId_t *pCmd = (struct RtblReqId_t* )pBuffer;

			RtblResData_t vRRt;
			vRRt.mNum = pConfig->GetRtblById(vRRt.mRtbl, pCmd->mId);
			if(vRRt.mNum > 0) SyncSend((char *)&vRRt, sizeof(vRRt));
			break;
		}
		case ASSERT_CMD(CMD_RTBL_REQ, RTBL_REQ_GID):	//id方式获取rtbl
		{
			RtblReqGid_t *pCmd = (struct RtblReqGid_t* )pBuffer;
			
			RtblResData_t vRRt;
			vRRt.mNum = pConfig->GetRtblByGid(vRRt.mRtbl, pCmd->mGid, 1);
			if(vRRt.mNum > 0) SyncSend((char *)&vRRt, sizeof(vRRt));
			break;
		}
		case ASSERT_CMD(CMD_RTBL_REQ, RTBL_REQ_NAME):	//id方式获取rtbl
		{
			RtblReqName_t *pCmd = (struct RtblReqName_t* )pBuffer;
			
			RtblResData_t vRRt;
			vRRt.mNum = pConfig->GetRtblByName(vRRt.mRtbl, pCmd->mName, 1);
			if(vRRt.mNum > 0) SyncSend((char *)&vRRt, sizeof(vRRt));
			break;
		}
		case ASSERT_CMD(CMD_RTBL_REQ, RTBL_REQ_GXID):
		{
			RtblReqGXid_t *pCmd = (struct RtblReqGXid_t* )pBuffer;
			
			RtblResData_t vRRt;
			vRRt.mNum = pConfig->GetRtblByGXid(vRRt.mRtbl, pCmd->mGid, pCmd->mXid, 1);
			if(vRRt.mNum > 0) SyncSend((char *)&vRRt, sizeof(vRRt));
			break;
		}
		default:
		{
			LOG_DEBUG("default", "client fd(%d) send a invalid msg id(%u)", mSocket->SocketFD(), pCommand->GetId());
			break;
		}
	}
	return 0;
}
