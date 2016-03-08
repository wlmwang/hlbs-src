
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_BASE_CMD_H_
#define _W_BASE_CMD_H_

#include "wType.h"
#include "wCommand.h"

#define USER  "nobody"
#define GROUP  "nobody"

//define
#define PREFIX  "/home/hlfs/disvr/routerserver/"
#define CONF_PREFIX  "/home/hlfs/disvr/routerserver/conf/"
 
#define PID_PATH  "/home/hlfs/disvr/routerserver/bin/disvr.pid"
#define LOCK_PATH  "/home/hlfs/disvr/routerserver/bin/disvr.lock"
#define WAIT_MUTEX	"/home/hlfs/disvr/routerserver/bin/wait_mutex.shm.ipc"

#define IPC_SHM "/home/hlfs/disvr/agent_cmd.shm.ipc"

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