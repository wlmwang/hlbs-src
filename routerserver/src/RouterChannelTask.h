
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _ROUTER_CHANNEL_TASK_H_
#define _ROUTER_CHANNEL_TASK_H_

#include <functional>

#include "wCore.h"
#include "wAssert.h"
#include "wLog.h"
#include "wIO.h"
#include "wTask.h"
#include "wChannelTask.h"
#include "wDispatch.h"
#include "Common.h"

#define CHANNEL_REG_DISP(cmdid, paraid, func) mDispatch.Register("RouterChannelTask", CMD_ID(cmdid, paraid), REG_FUNC(CMD_ID(cmdid, paraid), func));

class RouterChannelTask : public wChannelTask
{
	public:
		RouterChannelTask();
		RouterChannelTask(wIO *pIO);
		~RouterChannelTask();
		
		void Initialize();

		virtual int HandleRecvMessage(char * pBuffer, int nLen);
		int ParseRecvMessage(struct wCommand* pCommand, char *pBuffer, int iLen);

		DEC_FUNC(ChannelOpen);
		DEC_FUNC(ChannelClose);
		DEC_FUNC(ChannelQuit);
		DEC_FUNC(ChannelTerminate);
		
	protected:
		DEC_DISP(mDispatch);
};

#endif
