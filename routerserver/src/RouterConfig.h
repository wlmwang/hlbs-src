
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _ROUTER_CONFIG_H_
#define _ROUTER_CONFIG_H_

#include <string.h>
#include <vector>

#include "wType.h"
#include "wLog.h"
#include "wSingleton.h"

#include "Rtbl.h"

/**
 * 配置文件读取的数据结构
 */
class RouterConfig: public wSingleton<RouterConfig>
{
	public:
		char mExIPAddr[MAX_IP_LEN];
		unsigned int mExPort;
		unsigned int mBacklog;

		~RouterConfig() {}

		//初始化
		void Initialize()
		{
			memset(mExIPAddr, 0, sizeof(mExIPAddr));
			mExPort = 0;
			mBacklog = 1024;
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
		/*
		Rtbl_t GetRtblByName(string sName);
		Rtbl_t GetRtblById(int nId);
		Rtbl_t* GetRtblByGid(int nGid);
		Rtbl_t* GetRtblByGXid(int nGid, int nXid);		
		*/
	protected:
		vector<Rtbl_t> mRtbl;
};

#endif
