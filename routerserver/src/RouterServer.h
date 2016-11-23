
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

	virtual const wStatus& Run();
	virtual const wStatus& NewTcpTask(wSocket* sock, wTask** ptr);

	// 检测配置文件是否修改(增量同步)
    const wStatus& CheckModSvr();
};

#endif
