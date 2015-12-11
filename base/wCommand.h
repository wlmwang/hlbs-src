
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

/**
 *  消息头
 */
#ifndef _W_COMMAND_H_
#define _W_COMMAND_H_

#include <string.h>
#include "wType.h"

#pragma pack(1)

enum REQUEST_TYPE
{
	SERVER = 1,
	CLIENT,
};

const BYTE CMD_NULL = 0;		/** 空的指令 */
const BYTE PARA_NULL = 0;		/** 空的指令参数 */

struct wCommand
{
	BYTE mCmd;					/** 指令代码 */
	BYTE mPara;					/** 指令代码子编号 */

	BYTE GetCmd() const { return mCmd; }
	BYTE GetPara() const { return mPara; }
	
	wCommand(const BYTE cmd = CMD_NULL, const BYTE para = PARA_NULL) : mCmd(cmd), mPara(para) {};
};

#pragma pack()

#endif
