
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_TCP_TASK_H_
#define _W_TCP_TASK_H_

#include "wType.h"
#include "wTask.h"
#include "wSocket.h"
#include "wLog.h"
#include "wMisc.h"
#include "wCommand.h"

class wTcpTask : public wTask
{
	public:
		wTcpTask();
		wTcpTask(wIO *pIO);
		void Initialize();
		virtual ~wTcpTask();
		
		virtual int VerifyConn();	//验证接收到连接
		virtual int Verify();		//发送连接验证请求

		virtual int Heartbeat();
		virtual int HeartbeatOutTimes();
		
		virtual int HandleRecvMessage(char * pBuffer, int nLen) {}
		
		int ConnType() { return mConnType; }
	protected:
		int mConnType; //客户端类型
};

#endif
