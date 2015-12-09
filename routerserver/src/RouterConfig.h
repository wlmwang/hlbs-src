
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _ROUTER_CONFIG_H_
#define _ROUTER_CONFIG_H_

#include <string.h>
#include <vector>

#include "wType.h"
#include "wConfig.h"

#include "Rtbl.h"

/**
 * 配置文件读取的数据结构
 */
class RouterConfig: public wConfig<RouterConfig>
{
	public:
		char mIPAddr[MAX_IP_LEN];
		unsigned int mPort;
		unsigned int mBacklog;

		~RouterConfig() {}

		//初始化
		void Initialize()
		{
			memset(mIPAddr, 0, sizeof(mIPAddr));
			mPort = 0;
			mBacklog = 512;
		}

		RouterConfig()
		{
            Initialize();
		}
		
		/**
		 * 解析配置
		 */		
		void ParseBaseConfig();
		
		/**
		 *  解析Rtbl配置
		 */
		void ParseRtblConfig();
		
	protected:
		vector<Rtbl_t> mRtbl;
};

#endif
