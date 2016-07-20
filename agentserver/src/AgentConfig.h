
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _AGENT_CONFIG_H_
#define _AGENT_CONFIG_H_

#include <vector>
#include <map>

#include "wCore.h"
#include "wConfig.h"
#include "wMisc.h"
#include "wLog.h"
#include "wSingleton.h"
#include "SvrCmd.h"
#include "SvrQos.h"
#include "wMemPool.h"

#define MAX_ROUTER_NUM 64	//最多连接64个router

#define CONF_XML "../config/conf.xml"
#define ROUTER_XML "../config/router.xml"
#define QOS_XML "../config/qos.xml"

class AgentConfig : public wConfig<AgentConfig>
{
	public:
		//router配置，server.xml
		struct RouterConf_t
		{
			char mIPAddr[MAX_IP_LEN] {'\0'};
			unsigned int mPort {0};
			short mDisabled {0};
		};

		char mIPAddr[MAX_IP_LEN] {'\0'};
		unsigned int mPort {0};
		unsigned int mBacklog {LISTEN_BACKLOG};
		unsigned int mWorkers {1};
		
	public:
		AgentConfig();
		virtual ~AgentConfig();
		struct RouterConf_t* GetOneRouterConf();	//获取一个router服务器
		
		void GetBaseConf();
		void GetRouterConf();
		void GetQosConf();
		
		SvrQos *Qos() { return mSvrQos; }

	protected:
		struct RouterConf_t mRouterConf[MAX_ROUTER_NUM];

		SvrQos *mSvrQos {NULL};
		TiXmlDocument *mDoc {NULL};
		wMemPool *mMemPool {NULL};

		string mRouteConfFile = ROUTER_XML;
		string mQosConfFile = QOS_XML;
		string mBaseConfFile = CONF_XML;
};

#endif
