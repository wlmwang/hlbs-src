
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */
 
#include "RouterServerTask.h"
#include "RouterConfig.h"

#include "RouterCommand.h"

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
	switch(pCommand->GetCmd())
	{
		case CMD_NULL:
		{
			//空消息(心跳返回)
			mHeartbeatTimes = 0;
			mSocket->Stamp() = time(NULL);
			break;
		}
		case CMD_RTBL: //CMD_RTBL_BY_ID:
		{
			RtblById_t *pCmd = (RtblById_t* )pBuffer;
			Rtbl_t vRtbl = pConfig->GetRtblById(pCmd->mId);
			AsyncSend((char *)&vRtbl, sizeof(vRtbl));
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
