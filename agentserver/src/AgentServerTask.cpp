
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
	switch(pHeadCmd->mCmd)
	{
		//客户端登录Login请求跳转
		//case ID_C2S_REQUEST_LOGINLOGIC:
		//{
			//OnMessageLoginLogicRequest(vMessage, vClientFD);
		//	break;
		//}
		//case GET_RTBL:
		//{
			//RequestCmd stRequest = (RequestCmd)*pBuffer;
			//Rtbl_t stRtbl = RouterConfig::Instance()->GetRtbl(stRequest.mName);
			//int vRet = HandleSendMessage(&stRtbl,sizeof(stRtbl));
			//if(vRet < 0)
			//{
				//DisconnectClient(vClientFD);
			//}
		//}
		default:
		{
			//LOG_DEBUG("default", "client fd(%d) send a invalid msg id(%u), close it", vClientFD, stHeadCmd.mCmd);
			//DisconnectClient(vClientFD);
			break;
		}
	}
	return 0;
}
