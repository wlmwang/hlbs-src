
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _W_HTTP_TASK_H_
#define _W_HTTP_TASK_H_

#include "wCore.h"
#include "wTask.h"
#include "wCommand.h"

class wHttpTask : public wTask
{
	public:
		wHttpTask();
		wHttpTask(wIO *pIO);
		void Initialize();
		virtual ~wHttpTask();
		
		virtual int HandleRecvMessage(char * pBuffer, int nLen) {}
};

#endif
