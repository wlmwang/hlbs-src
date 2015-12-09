
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */
 
#include "RouterServerTask.h"

#include "wHeadCmd.h"
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
	struct wHeadCmd *pHeadCmd = (struct wHeadCmd*) pBuffer;
	return ParseRecvMessage(pHeadCmd , pBuffer, nLen);
}

int RouterServerTask::ParseRecvMessage(struct wHeadCmd *pHeadCmd, char *pBuffer, int iLen)
{
	switch(pHeadCmd->mCommand.GetCmdType())
	{
		//pHeadCmd->mCommand.GetParaType()
		
		
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
