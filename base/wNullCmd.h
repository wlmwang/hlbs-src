
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

/**
 * 负责服务器内部交换使用，和客户端交互的指令需要另外定义
 */
#ifndef _W_NULL_CMD_H_
#define _W_NULL_CMD_H_

#include "wType.h"

#pragma pack(1)

const BYTE CMD_NULL = 0;		/**< 空的指令 */
const BYTE PARA_NULL = 0;		/**< 空的指令参数 */

struct NullCmd_t
{
	BYTE cmd;					/**< 指令代码 */
	BYTE para;					/**< 指令代码子编号 */

	BYTE GetCmdType() const { return cmd; }
	BYTE GetParaType() const { return para; }
	
	NullCmd_t(const BYTE cmd = CMD_NULL, const BYTE para = PARA_NULL) : cmd(cmd), para(para) {};
};

//socket服务端空指令子编号
const BYTE SERVER_PARA_NULL = 0;

//socket服务端空操作指令，测试信号和对时间指令
struct ServerNullCmd_t:public NullCmd_t
{
	ServerNullCmd_t():NullCmd_t(CMD_NULL,SERVER_PARA_NULL) {}
};

//socket客户端空指令子编号
const BYTE CLIENT_PARA_NULL = 1;

//socket客户端空操作指令，测试信号和对时间指令
struct ClientNullCmd_t:public NullCmd_t
{
	ClientNullCmd_t():NullCmd_t(CMD_NULL,CLIENT_PARA_NULL) {}
};

#pragma pack()

#endif

