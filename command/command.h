
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_COMMAND_H_
#define _W_COMMAND_H_

#include "wType.h"

//服务器ID
enum SERVER_TYPE
{
	SERVER_ROUTER = 1,
	SERVER_AGENT,
};

#pragma pack(1)

namespace Cmd
{
	const BYTE CMD_NULL = 0;		/** 空的指令 */
	const BYTE PARA_NULL = 0;		/** 空的指令参数 */
	
	struct Command
	{
		BYTE cmd;					/** 指令代码 */
		BYTE para;					/** 指令代码子编号 */

		BYTE GetCmdType() const { return cmd; }
		BYTE GetParaType() const { return para; }
		
		Command(const BYTE cmd = CMD_NULL, const BYTE para = PARA_NULL) : cmd(cmd), para(para) {};
	};
};

#pragma pack()

#endif