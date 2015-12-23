
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_BASE_COMMAND_H_
#define _W_BASE_COMMAND_H_

#include "wCommand.h"

#pragma pack(1)

//连接类型（对应command中）
enum
{
	CLIENT_USER = 1,
	SERVER_ROUTER,
	SERVER_AGENT,
	SERVER_CMD,
};

#pragma pack()

#endif