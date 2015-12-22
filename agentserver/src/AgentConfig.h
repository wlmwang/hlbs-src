
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
#include "AgentServer.h"

#include "RtblCommand.h"

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
		~AgentConfig() 
		{
			CleanRtbl();
		}
		
		void CleanRtbl();
		
		/**
		 * 解析配置
		 */		
		void ParseBaseConfig();
		
		int GetAllRtblReq();
		void FixRtbl(Rtbl_t pRtbl[] = NULL, int iLen = 0); //整理容器
		
		int GetRtblById(Rtbl_t* pBuffer, int iId);
		int GetRtblAll(Rtbl_t* pBuffer, int iNum = 1);
		int GetRtblByName(Rtbl_t* pBuffer, string sName, int iNum = 1);
		int GetRtblByGid(Rtbl_t* pBuffer, int iGid, int iNum = 1);
		int GetRtblByGXid(Rtbl_t* pBuffer, int iGid, int iXid, int iNum = 1);
		
		BYTE SetRtblAttr(WORD iId, BYTE iDisabled, WORD iWeight, WORD iTimeline, WORD iConnTime, WORD iTasks, WORD iSuggest);
	protected:
		BYTE DisabledRtbl(WORD iId);
		BYTE SetRtblWeight(WORD iId, WORD iWeight);
		
		vector<Rtbl_t*> mRtbl;
		map<int, Rtbl_t*> mRtblById;
		map<int, vector<Rtbl_t*> > mRtblByGid;
		map<string, vector<Rtbl_t*> > mRtblByName;
		map<string, vector<Rtbl_t*> > mRtblByGXid;
};

#endif
