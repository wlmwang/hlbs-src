
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _ROUTER_CONFIG_H_
#define _ROUTER_CONFIG_H_

#include <string.h>
#include <vector>

#include "wConfig.h"
#include "wCore.h"
#include "wLog.h"
#include "wSingleton.h"
#include "AgentCmd.h"

/**
 * 配置文件读取的数据结构
 */
class AgentCmdConfig: public wConfig<AgentCmdConfig>
{
	public:
		char mIPAddr[MAX_IP_LEN];
		unsigned int mPort;
		
		~AgentCmdConfig() {}

		//初始化
		void Initialize()
		{
			memset(mIPAddr, 0, sizeof(mIPAddr));
			mPort = 0;
			mDoc = new TiXmlDocument();
		}

		AgentCmdConfig()
		{
            Initialize();
		}
		
		/**
		 * 解析配置
		 */		
		void ParseBaseConfig();

	protected:
		TiXmlDocument* mDoc;
};

#endif
