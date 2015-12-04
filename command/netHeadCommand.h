
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_NET_HEAD_COMMAND_H_
#define _W_NET_HEAD_COMMAND_H_

#include <string.h>
#include "wType.h"

#include "command.h"

#pragma pack(1)

//客户端的消息类型
enum CLIENT_MSG_TYPE
{
	TYPE_LOGIC	= 1;			//逻辑控制消息
	TYPE_STREAM	= 2;			//流消息
	TYPE_SOUND	= 3;			//声音消息
};

struct NetHeadCommand_t : Command
{
	DWORD	mClientIp;		//客户端的IP地址
	DWORD	mClientPort;	//客户端的端口
	DWORD	mSockTime;		//socket创建时间
	DWORD	mTimeStamp;		//接收到此次消息的时间戳
	DWORD	mClientState;	//客户端当前状态，=0表示一切正常，<0表示客户端已经断开连接，>0表示注册消息，这时mClientFD代表服务器的类型
	//DWORD	mClientFD;
};

#pragma pack()

#endif