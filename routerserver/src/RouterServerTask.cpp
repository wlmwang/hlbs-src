
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */
 
#include "RouterServerTask.h"
#include "RouterConfig.h"

#include "RouterCommand.h"
#include "AgentCommand.h"

#include "wAssert.h"

RouterServerTask::RouterServerTask(wSocket *pSocket) : wTcpTask(pSocket)
{
    //...
}

RouterServerTask::~RouterServerTask()
{
    //...
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
	RouterConfig *pConfig = RouterConfig::Instance();
	switch(pCommand->GetCmd()<<8 | pCommand->GetPara())
	{
		case CMD_NULL<<8 | PARA_NULL:
		{
			//空消息(心跳返回)
			mHeartbeatTimes = 0;
			break;
		}
		case CMD_RTBL_REQUEST<<8 | CMD_RTBL_ALL:
		{
			RtblAll_t *pCmd = (RtblAll_t* )pBuffer;
			RtblResponse_t vRRt;
			vRRt.mNum = pConfig->GetRtblAll(vRRt.mRtbl, 0);
			
			SyncSend((char *)&vRRt, sizeof(vRRt));
			break;
		}
		case CMD_RTBL_REQUEST<<8 | CMD_RTBL_BY_ID:
		{
			RtblById_t *pCmd = (RtblById_t* )pBuffer;
			RtblResponse_t vRRt;
			vRRt.mNum = 1;
			Rtbl_t vRtbl = pConfig->GetRtblById(pCmd->mId);
			memcpy(vRRt.mRtbl, (char* )&vRtbl , sizeof(vRtbl));
			SyncSend((char *)&vRRt, sizeof(vRRt));
			break;
		}
		case CMD_RTBL_REQUEST<<8 | CMD_RTBL_BY_GID:
		{
			RtblByGid_t *pCmd = (RtblByGid_t* )pBuffer;
			
			RtblResponse_t vRRt;
			vRRt.mNum = pConfig->GetRtblByGid(vRRt.mRtbl, pCmd->mGid, 1);
			if(vRRt.mNum) SyncSend((char *)&vRRt, sizeof(vRRt));
			break;
		}
		case CMD_RTBL_REQUEST<<8 | CMD_RTBL_BY_NAME:
		{
			RtblByName_t *pCmd = (RtblByName_t* )pBuffer;
			
			RtblResponse_t vRRt;
			vRRt.mNum = pConfig->GetRtblByName(vRRt.mRtbl, pCmd->mName, 1);
			if(vRRt.mNum > 0) SyncSend((char *)&vRRt, sizeof(vRRt));
			break;
		}
		case CMD_RTBL_REQUEST<<8 | CMD_RTBL_BY_GXID:
		{
			RtblByGXid_t *pCmd = (RtblByGXid_t* )pBuffer;
			
			RtblResponse_t vRRt;
			vRRt.mNum = pConfig->GetRtblByGXid(vRRt.mRtbl, pCmd->mGid, pCmd->mXid, 1);
			if(vRRt.mNum > 0) SyncSend((char *)&vRRt, sizeof(vRRt));
			break;
		}
		default:
		{
			//LOG_DEBUG("default", "client fd(%d) send a invalid msg id(%u), close it", vClientFD, stHeadCmd.mCmd);
			//DisconnectClient(vClientFD);
			break;
		}
	}
	return 0;
}
