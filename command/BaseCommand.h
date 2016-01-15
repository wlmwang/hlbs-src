
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_BASE_COMMAND_H_
#define _W_BASE_COMMAND_H_

#include "wCommand.h"

#define SVR_SHARE_MEM_PIPE "/tmp/svr_shm"

#pragma pack(1)

//连接类型
enum
{
	CLIENT_USER = 2,
	SERVER_ROUTER,
	SERVER_AGENT,
	SERVER_CMD,
};

#pragma pack()

#endif