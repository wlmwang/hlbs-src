
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_ROUTER_COMMAND_H_
#define _W_ROUTER_COMMAND_H_

#include <string.h>
#include "wCommand.h"
#include "Rtbl.h"

#pragma pack(1)

const BYTE CMD_RTBL_REQUEST = 10;
struct RtblRequestCmd_s : public wCommand
{
	RtblRequestCmd_s(BYTE para)
	{
		mCmd = CMD_RTBL_REQUEST;
		mPara = para;
	}
};

const BYTE CMD_RTBL_ALL = 0;
struct RtblAll_t : RtblRequestCmd_s 
{
	RtblAll_t() : RtblRequestCmd_s(CMD_RTBL_ALL) {}
};

const BYTE CMD_RTBL_BY_ID = 1;
struct RtblById_t : RtblRequestCmd_s 
{
	RtblById_t() : RtblRequestCmd_s(CMD_RTBL_BY_ID), mId(0) {}
	
	WORD mId;
};

const BYTE CMD_RTBL_BY_GID = 2;
struct RtblByGid_t : RtblRequestCmd_s 
{
	RtblByGid_t() : RtblRequestCmd_s(CMD_RTBL_BY_GID), mGid(0) {}
	
	WORD mGid;
};

const BYTE CMD_RTBL_BY_NAME = 3;
struct RtblByName_t : RtblRequestCmd_s 
{
	RtblByName_t() : RtblRequestCmd_s(CMD_RTBL_BY_NAME) 
	{
		memset(mName, 0, 64);
	}
	
	char mName[64];
};

const BYTE CMD_RTBL_BY_GXID = 4;
struct RtblByGXid_t : RtblRequestCmd_s 
{
	RtblByGXid_t() : RtblRequestCmd_s(CMD_RTBL_BY_GXID), mGid(0),mXid(0) {}
	
	WORD mGid;
	WORD mXid;
};

#pragma pack()

#endif