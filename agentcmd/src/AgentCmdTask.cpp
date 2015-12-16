
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */
 
#include "AgentCmdTask.h"
#include "AgentCmdConfig.h"

#include "wAssert.h"

AgentCmdTask::AgentCmdTask(wSocket *pSocket):wTcpTask(pSocket)
{
    //...
}

AgentCmdTask::~AgentCmdTask()
{
    //...
}

/**
 *  接受数据
 */
int AgentCmdTask::HandleRecvMessage(char * pBuffer, int nLen)
{
	W_ASSERT(pBuffer != NULL, return -1);
	//解析消息
	struct wCommand *pCommand = (struct wCommand*) pBuffer;
	return ParseRecvMessage(pCommand , pBuffer, nLen);
}

int AgentCmdTask::ParseRecvMessage(struct wCommand* pCommand, char *pBuffer, int iLen)
{
	AgentCmdConfig *pConfig = AgentCmdConfig::Instance();
	switch(pCommand->GetId())
	{
		case ASSERT_CMD(CMD_NULL, PARA_NULL):
		{
			//空消息(心跳返回)
			mHeartbeatTimes = 0;
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
