
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_BASE_CMD_H_
#define _W_BASE_CMD_H_

#include "wType.h"
#include "wCommand.h"

//define
#define PREFIX  "/usr/local/disvr/"
#define CONF_PREFIX  "conf/"

#define PID_PATH  "log/disvr.pid"
#define LOCK_PATH  "log/disvr.lock"

#define IPC_SHM "/tmp/shm.ipc"

#define USER  "nobody"
#define GROUP  "nobody"

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