
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */
 
#include "RouterClientTask.h"
#include "RouterClient.h"

#include "wHeadCmd.h"
#include "wAssert.h"

RouterClientTask::RouterClientTask(int iNewSocket, struct sockaddr_in *stSockAddr):wTcpTask(iNewSocket,stSockAddr)
{
    //...
}

RouterClientTask::~RouterClientTask()
{
    //
}

/**
 *  接受数据
 */
int RouterClientTask::HandleRecvMessage(char * pBuffer, int nLen)
{
	W_ASSERT(pBuffer != NULL, return false);

	return 0;
	//return RouterServer::Instance()->mInMsgQueue.PutOneMsg(pBuffer, nLen);
}

/**
 *  发送数据
 */
int RouterClientTask::HandleSendMessage(char *pBuffer, int nLen)
{
	W_ASSERT(pBuffer != NULL, return -1);
	W_ASSERT(nLen > 0 && nLen < MAX_CLIENT_MSG_LEN, return -1);	//100k
	
	/*
	if(!RouterServer::Instance()->mOutMsgQueue.PutOneMsg(pBuffer, nLen))
	{
		return -1;
	}
	*/
	return 0;
}

/**
 *  发送数据 daemon
 */
void RouterClientTask::ProcessSendMessage()
{
	static char szBuff[MAX_PACKAGE_LEN] = {0};
	int iRet;
	while(1)
	{
		//iRet = RouterServer::Instance()->mOutMsgQueue.GetOneMsg(szBuff, MAX_PACKAGE_LEN);
		//没有消息了
		if(iRet == 0) 
		{
			return;
		}

		//取消息出错
		if(iRet < 0) 
		{
			LOG_ERROR("default", "get one message from msg queue failed: %d", iRet);
			return;
		}

		// 如果消息大小不正确
		if(iRet < MIN_CLIENT_MSG_LEN || iRet > MAX_CLIENT_MSG_LEN) 
		{
			LOG_ERROR("default", "get a msg with invalid len %d from main server", iRet);
			return;
		}

		int iLen = *(int *)szBuff;
		W_ASSERT(iRet == iLen + sizeof(int), return);
		
		char *pBuffer = ((char *)szBuff) + sizeof(int);
		SendBytes(pBuffer, iLen);
	}
}

/**
 *  处理接受缓冲区数据 daemon
 */
void RouterClientTask::ProcessRecvMessage()
{
	static char szBuff[MAX_PACKAGE_LEN] = {0};
	int iRet;
	while(1)
	{
		//iRet = RouterServer::Instance()->mInMsgQueue.GetOneMsg(szBuff, MAX_PACKAGE_LEN);
		//没有新消息
		if(iRet == 0) 
		{
			return;
		}

		//如果出错
		if(iRet < 0) 
		{
			LOG_ERROR("default", "get one message from msg queue failed: %d", iRet);
			return;
		}

		//如果长度不正常
		if(iRet < MIN_CLIENT_MSG_LEN || iRet > MAX_CLIENT_MSG_BUFFER_LEN) 
		{
			LOG_ERROR("default", "get a msg from msg queue with invalid len: %d", iRet);
			return;
		}
		
		int iLen = *(int *)szBuff;
		W_ASSERT(iRet == iLen + sizeof(int), return);
		
		char *pBuffer = ((char *)szBuff) + sizeof(int);
		//解析消息
		struct wHeadCmd *pHeadCmd = (struct wHeadCmd*) pBuffer;
		ParseRecvMessage(pHeadCmd ,(char *)pBuffer, iLen);
	}
}

void RouterClientTask::ParseRecvMessage(struct wHeadCmd *pHeadCmd, char *pBuffer, int iLen)
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
}
