
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_LOGIN_COMMAND_H_
#define _W_LOGIN_COMMAND_H_

#include <string.h>
#include "BaseCommand.h"

#pragma pack(1)

//++++++++++++请求数据结构
const BYTE CMD_LOGIN_REQ = 1;
struct LoginReqCmd_s : public wCommand
{
	LoginReqCmd_s(BYTE para)
	{
		mCmd = CMD_LOGIN_REQ;
		mPara = para;
	}
};

const BYTE LOGIN_REQ_TOKEN = 0;
struct LoginReqToken_t : LoginReqCmd_s 
{
	LoginReqToken_t() : LoginReqCmd_s(LOGIN_REQ_TOKEN)
	{
		memset(mToken, 0, 32);
	}
	BYTE mConnType;
	char mToken[32];
};


//++++++++++++返回
const BYTE CMD_LOGIN_RES = 2;
struct LoginResCmd_s : public wCommand
{
	LoginResCmd_s(BYTE para)
	{
		mCmd = CMD_LOGIN_RES;
		mPara = para;
	}
};

const BYTE LOGIN_RES_STATUS = 0;
struct LoginResStatus_t : LoginResCmd_s 
{
	LoginResStatus_t() : LoginResCmd_s(LOGIN_RES_STATUS)
	{
		mStatus = 0;
	}
	
	char mStatus;
};

#pragma pack()

#endif