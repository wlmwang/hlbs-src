
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_BASE_COMMAND_H_
#define _W_BASE_COMMAND_H_

#define PREFIX  "/usr/local/disvr/"

#define CONF_PREFIX  "conf/"

#define PID_PATH  "log/disvr.pid"

#define LOCK_PATH  "log/disvr.lock"

#define USER  "nobody"			//默认用户名
#define GROUP  "nobody"			//默认用户组

#define IPC_SHM "/tmp/shm.ipc"

#include "wCommand.h"

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