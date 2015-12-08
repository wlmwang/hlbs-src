
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */
 
#include "AgentServerTask.h"

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
	struct wHeadCmd *pHeadCmd = (struct wHeadCmd*) pBuffer;
	return ParseRecvMessage(pHeadCmd , pBuffer - sizeof(struct wHeadCmd), nLen - sizeof(struct wHeadCmd));
}

int AgentServerTask::ParseRecvMessage(struct wHeadCmd *pHeadCmd, char *pBuffer, int iLen)
{
	return 0;
}
