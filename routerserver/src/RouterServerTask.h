
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _ROUTER_SERVER_TASK_H_
#define _ROUTER_SERVER_TASK_H_

#include <arpa/inet.h>

#include "wType.h"
#include "wCommand.h"
#include "wTcpTask.h"
#include "wLog.h"

class RouterServerTask : public wTcpTask
{
	public:
		RouterServerTask() {}
		~RouterServerTask();
		RouterServerTask(wSocket *pSocket);
		
		virtual int HandleRecvMessage(char * pBuffer, int nLen);
		
		int ParseRecvMessage(struct wCommand* pCommand, char *pBuffer, int iLen);
};

#endif
