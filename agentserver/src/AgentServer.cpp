
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
	HNET_NEW(AgentTcpTask(sock), *ptr);
    if (!*ptr) {
        HNET_ERROR(soft::GetLogPath(), "%s : %s", "RouterServer::NewTcpTask new() failed", "");
        return -1;
    }
	return 0;
}

int AgentServer::NewUnixTask(wSocket* sock, wTask** ptr) {
	HNET_NEW(AgentUnixTask(sock), *ptr);
    if (!*ptr) {
        HNET_ERROR(soft::GetLogPath(), "%s : %s", "RouterServer::NewUnixTask new() failed", "");
        return -1;
    }
	return 0;
}

int AgentServer::NewChannelTask(wSocket* sock, wTask** ptr) {
	HNET_NEW(AgentChannelTask(sock, Master<AgentMaster*>()), *ptr);
    if (!*ptr) {
        HNET_ERROR(soft::GetLogPath(), "%s : %s", "RouterServer::NewChannelTask new() failed", "");
        return -1;
    }
    return 0;
}
