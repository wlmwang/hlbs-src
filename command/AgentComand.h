
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_AGENT_COMMAND_H_
#define _W_AGENT_COMMAND_H_

#include <string.h>
#include "wCommand.h"

#pragma pack(1)

const BYTE CMD_RTBL_RESPONSE = 11;
struct RtblResponseCmd_s : public wCommand
{
	RtblResponseCmd_s(BYTE para)
	{
		mCmd = CMD_RTBL_RESPONSE;
		mPara = para;
	}
};

const BYTE CMD_RTBL_R = 0;
struct RtblResponse_t : RtblResponseCmd_s 
{
	RtblResponse_t() : RtblResponseCmd_s(CMD_RTBL_R) 
	{
		mNum = 0;
		memset(mName, 0, 256);
	}
	Rtbl mRtbl[256];
	int mNum;
};

#pragma pack()

#endif