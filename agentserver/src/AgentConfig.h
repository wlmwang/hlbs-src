
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _ROUTER_CONFIG_H_
#define _ROUTER_CONFIG_H_

#include <string.h>
#include <vector>

#include "wConfig.h"
#include "wType.h"
#include "wLog.h"
#include "wSingleton.h"

/**
 * 配置文件读取的数据结构
 */
class AgentConfig: public wConfig<AgentConfig>
{
	public:
		char mIPAddr[MAX_IP_LEN];
		unsigned int mPort;
		unsigned int mBacklog;
		
		char mRouterIPAddr[MAX_IP_LEN];
		unsigned int mRouterPort;
		
		~AgentConfig() {}

		//初始化
		void Initialize()
		{
			memset(mIPAddr, 0, sizeof(mIPAddr));
			mPort = 0;
			mBacklog = 512;
			
			memset(mRouterIPAddr, 0, sizeof(mRouterIPAddr));
			mRouterPort = 0;
		}

		AgentConfig()
		{
            Initialize();
		}
		
		/**
		 * 解析配置
		 */		
		void ParseBaseConfig();
};

#endif
