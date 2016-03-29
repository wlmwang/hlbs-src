
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

		char mIPAddr[MAX_IP_LEN];
		unsigned int mPort;
		unsigned int mBacklog;
		unsigned int mWorkers;
	
	public:
		AgentConfig();
		virtual ~AgentConfig();
		void Initialize();
		struct RouterConf_t* GetOneRouterConf();	//获取一个router服务器
		
		void GetBaseConf();
		void GetRouterConf();
		void GetQosConf();
		
		SvrQos *Qos() { return mSvrQos; }
		
	protected:
		SvrQos *mSvrQos;
		TiXmlDocument* mDoc;
		wMemPool *mMemPool;
		
		RouterConf_t mRouterConf[MAX_ROUTER_NUM];

		char mRouteConfFile[255];
		char mQosConfFile[255];
		char mBaseConfFile[255];
};

#endif
