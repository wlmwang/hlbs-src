
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _ROUTER_SERVER_TASK_H_
#define _ROUTER_SERVER_TASK_H_

#include <arpa/inet.h>

#include "wType.h"
#include "wHeadCmd.h"
#include "wTcpTask.h"
#include "wLog.h"

class RouterServerTask : public wTcpTask
{
	public:
		RouterServerTask(int iNewSocket, struct sockaddr_in *stSockAddr);
		
        ~RouterServerTask();
		
		virtual int HandleRecvMessage(char * pBuffer, int nLen);
		virtual void ProcessSendMessage();

		virtual int HandleSendMessage(char * pBuffer, int nLen);
		virtual void ProcessRecvMessage();
		virtual void ParseRecvMessage(struct wHeadCmd* pHeadCmd ,char *pBuffer,int iLen);
};

#endif
