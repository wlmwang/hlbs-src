
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _W_UDSOCKET_TASK_H_
#define _W_UDSOCKET_TASK_H_

#include "wCore.h"
#include "wTask.h"
#include "wCommand.h"

class wUDSocketTask : public wTask
{
	public:
		wUDSocketTask();
		wUDSocketTask(wIO *pIO);
		void Initialize();
		virtual ~wUDSocketTask();
		
		virtual int VerifyConn();	//验证接收到连接
		virtual int Verify();		//发送连接验证请求

		int ConnType() { return mConnType; }
	protected:
		int mConnType; //客户端类型
};

#endif
