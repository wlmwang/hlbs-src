
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _ROUTER_SERVER_H_
#define _ROUTER_SERVER_H_

#include "wCore.h"
#include "wStatus.h"
#include "wMisc.h"
#include "wServer.h"

using namespace hnet;

class RouterServer: public wServer {
public:
	RouterServer(wConfig* config) : wServer(config) { }

	virtual const wStatus& PrepareRun();
	virtual const wStatus& Run();
	virtual const wStatus& NewTcpTask(wSocket* sock, wTask** ptr);
	virtual const wStatus& NewHttpTask(wSocket* sock, wTask** ptr);
	virtual const wStatus& NewChannelTask(wSocket* sock, wTask** ptr);
};

#endif
