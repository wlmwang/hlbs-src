
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_ROUTER_COMMAND_H_
#define _W_ROUTER_COMMAND_H_

#include "wCommand.h"

#pragma pack(1)

//服务器ID
enum SERVER_TYPE
{
	SERVER_ROUTER = 1,
	SERVER_AGENT,
};

/*
//Agent->Router的消息体
const BYTE CMD_LOGIN          = 1;  //登录、连接指令
struct LoginCmd_s : public wCommand
{
	LoginCmd_s(BYTE para)
	{
		mCommand.mCmd = CMD_LOGIN;
		mCommand.mPara = para;
	}
};

//Agent登录Router连接
const BYTE PARA_SERVER_LOGIN = 1;
struct LoginFromS_t : LoginCmd_s
{
	LoginFromS_t() : LoginCmd_s(PARA_SERVER_LOGIN), wdServerID(0), wdServerType(0) {}

	WORD wdServerID;
	WORD wdServerType;
};
*/

const BYTE CMD_RTBL          = 10;
struct RtblCmd_s : public wCommand
{
	RtblCmd_s(BYTE para)
	{
		mCommand.mCmd = CMD_RTBL;
		mCommand.mPara = para;
	}
};

const BYTE CMD_RTBL_BY_ID = 1;
struct RtblById_t : RtblCmd_s 
{
	RtblById_t() : RtblCmd_s(CMD_RTBL_BY_ID), mId(0) {}
	
	WORD mId;
}

#pragma pack()

#endif