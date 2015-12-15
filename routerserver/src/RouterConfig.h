
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _ROUTER_CONFIG_H_
#define _ROUTER_CONFIG_H_

#include <string.h>
#include <string>
#include <map>
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
		
		Rtbl_t  GetRtblById(int iId);
		
		int GetRtblAll(Rtbl_t* pBuffer, int iNum = 1);
		int GetRtblByName(Rtbl_t* pBuffer, string sName, int iNum = 1);
		int GetRtblByGid(Rtbl_t* pBuffer, int iGid, int iNum = 1);
		int GetRtblByGXid(Rtbl_t* pBuffer, int iGid, int iXid, int iNum = 1);
		
	protected:
		void FixRtbl();	//整理容器
	
		vector<Rtbl_t*> mRtbl;
		map<int, Rtbl_t*> mRtblById;
		map<int, vector<Rtbl_t*> > mRtblByGid;
		map<string, vector<Rtbl_t*> > mRtblByName;
		map<string, vector<Rtbl_t*> > mRtblByGXid;
};

#endif
