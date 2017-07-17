
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _ROUTER_SERVER_H_
#define _ROUTER_SERVER_H_

#include "wCore.h"
#include "wMisc.h"
#include "wServer.h"
#include "wTask.h"
#include "SvrCmd.h"
#include "AgntCmd.h"
#include "RouterConfig.h"
#include "Misc.h"

using namespace hnet;

class RouterServer: public wServer {
public:
	RouterServer(wConfig* config) : wServer(config) { }

	virtual int PrepareRun();
	virtual int Run();
	virtual int NewTcpTask(wSocket* sock, wTask** ptr);
	virtual int NewHttpTask(wSocket* sock, wTask** ptr);
	virtual int NewChannelTask(wSocket* sock, wTask** ptr);

	template<typename T>
	int BroadcastSvr(char *cmd, int len);
};

// 尽量不改数据接口解决方案
// 正常应该修改command的SvrResCmd_s结构
// T 适用 SvrResReload_t SvrResSync_t
template<typename T>
int RouterServer::BroadcastSvr(char *cmd, int len) {
    RouterConfig* config = Config<RouterConfig*>();

    std::string host;
    for (std::vector<wTask*>::iterator it = mTaskPool.begin(); it != mTaskPool.end(); it++) {
        if ((*it)->Socket()->ST() == kStConnect && (*it)->Socket()->SS() == kSsConnected && (*it)->Socket()->SP() == kSpTcp && 
            ((*it)->Socket()->SF() == kSfSend || (*it)->Socket()->SF() == kSfRvsd)) {
            
            // 过滤节点
            host = FilterLocalIp((*it)->Socket()->Host()); // 客户端agent地址
            if (!config->IsExistRlt(host)) {
                continue;
            }

            // 过滤svr节点
            T r = *(reinterpret_cast<T*>(cmd));
            r.mNum = config->Qos()->FilterSvrBySid(r.mSvr, r.mNum, config->Rlts(host));
            if (r.mNum <= 0) {
                continue;
            }
            Send(*it, reinterpret_cast<char*>(&r), len);
        }
    }

    return 0;
}

#endif
