
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "AgentServer.h"
#include "AgentMaster.h"
#include "AgentTcpTask.h"
#include "AgentUnixTask.h"
#include "AgentChannelTask.h"

int AgentServer::NewTcpTask(wSocket* sock, wTask** ptr) {
	SAFE_NEW(AgentTcpTask(sock, Shard(sock)), *ptr);
    if (!*ptr) {
        H_LOG_ERROR(soft::GetLogPath(), "%s : %s", "RouterServer::NewTcpTask new() failed", "");
        return -1;
    }
	return 0;
}

int AgentServer::NewUnixTask(wSocket* sock, wTask** ptr) {
	SAFE_NEW(AgentUnixTask(sock, Shard(sock)), *ptr);
    if (!*ptr) {
        H_LOG_ERROR(soft::GetLogPath(), "%s : %s", "RouterServer::NewUnixTask new() failed", "");
        return -1;
    }
	return 0;
}

int AgentServer::NewChannelTask(wSocket* sock, wTask** ptr) {
	SAFE_NEW(AgentChannelTask(sock, Master<AgentMaster*>(), Shard(sock)), *ptr);
    if (!*ptr) {
        H_LOG_ERROR(soft::GetLogPath(), "%s : %s", "RouterServer::NewChannelTask new() failed", "");
        return -1;
    }
    return 0;
}
