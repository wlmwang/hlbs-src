
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _ROUTER_SERVER_H_
#define _ROUTER_SERVER_H_

#include "wCore.h"
#include "wMisc.h"
#include "wServer.h"

using namespace hnet;

class RouterServer: public wServer {
public:
	RouterServer(wConfig* config) : wServer(config) { }

	virtual int PrepareRun();
	virtual int Run();
	virtual int NewTcpTask(wSocket* sock, wTask** ptr);
	virtual int NewHttpTask(wSocket* sock, wTask** ptr);
	virtual int NewChannelTask(wSocket* sock, wTask** ptr);
};

#endif
