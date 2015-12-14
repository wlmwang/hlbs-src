
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_ROUTER_COMMAND_H_
#define _W_ROUTER_COMMAND_H_

#include "wCommand.h"

#pragma pack(1)

const BYTE CMD_RTBL = 10;
struct RtblCmd_s : public wCommand
{
	RtblCmd_s(BYTE para)
	{
		mCmd = CMD_RTBL;
		mPara = para;
	}
};

const BYTE CMD_RTBL_BY_ID = 1;
struct RtblById_t : RtblCmd_s 
{
	RtblById_t() : RtblCmd_s(CMD_RTBL_BY_ID), mId(0) {}
	
	WORD mId;
};

#pragma pack()

#endif