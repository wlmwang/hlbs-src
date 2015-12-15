
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */
 
#include "AgentServerTask.h"

#include "wAssert.h"
#include "AgentConfig.h"

#include "AgentCommand.h"
#include "RouterCommand.h"

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
	switch(pCommand->GetCmd()<<8 | pCommand->GetPara())
	{
		case CMD_NULL<<8 | PARA_NULL:
		{
			//空消息(心跳返回)
			mHeartbeatTimes = 0;
			break;
		}
		case CMD_RTBL_RESPONSE<<8 | CMD_RTBL_R:
		{
			RtblResponse_t *pCmd = (RtblResponse_t* )pBuffer;

			//cout <<"gid:"<< (int)pCmd->mRtbl[0].mGid<< pCmd->mRtbl[0].mName<<endl;
			//cout <<"gid:"<< (int)pCmd->mRtbl[1].mGid<< pCmd->mRtbl[1].mName<<endl;
			
			//pConfig->ResponseRtbl(pCmd, iLen);
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
