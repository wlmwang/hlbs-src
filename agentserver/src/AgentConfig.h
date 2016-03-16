
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
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

#define MAX_ROUTER_NUM 32

#define CONF_XML "../config/conf.xml"
#define ROUTER_XML "../config/router.xml"

//router配置，server.xml
struct RouterConf_t
{
	char mIPAddr[MAX_IP_LEN];
	unsigned int mPort;
	short mDisabled;
	RouterConf_t()
	{
		memset(mIPAddr, 0, sizeof(mIPAddr));
		mPort = 0;
		mDisabled = 0;
	}
};

class AgentConfig : public wConfig<AgentConfig>
{
	public:
		char mIPAddr[MAX_IP_LEN];
		unsigned int mPort;
		unsigned int mBacklog;
		unsigned int mWorkers;

		AgentConfig();
		virtual ~AgentConfig();
		void Initialize();
		struct RouterConf_t* GetOneRouterConf();	//获取一个router服务器
		
		void GetBaseConf();
		void GetRouterConf();

	protected:
		TiXmlDocument* mDoc;
		RouterConf_t mRouterConf[MAX_ROUTER_NUM];
};

#endif
