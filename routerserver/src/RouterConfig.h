
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
#include "wLog.h"
#include "Svr.h"

/**
 * 配置文件读取的数据结构
 */
class RouterConfig: public wConfig<RouterConfig>
{
	public:
		char mIPAddr[MAX_IP_LEN];
		unsigned int mPort;
		unsigned int mBacklog;
		unsigned int mWorkers;

		void Final();
		virtual ~RouterConfig() 
		{
			Final();
		}

		//初始化
		void Initialize()
		{
			memset(mIPAddr, 0, sizeof(mIPAddr));
			mPort = 0;
			mBacklog = 1024;
			mWorkers = 1;
			mDoc = new TiXmlDocument();
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
		 *  解析Svr配置
		 */
		void ParseSvrConfig();
		
		int ReloadSvr(Svr_t* pBuffer, int iNum = 0);
		int SyncSvr(Svr_t* pBuffer, int iNum = 0);
		int GetSvrAll(Svr_t* pBuffer, int iNum = 0);
		int GetSvrByName(Svr_t* pBuffer, string sName, int iNum = 0);
		int GetSvrByGid(Svr_t* pBuffer, int iGid, int iNum = 0);
		int GetSvrByGXid(Svr_t* pBuffer, int iGid, int iXid, int iNum = 0);
		int GetSvrById(Svr_t* pBuffer, int iId);
	protected:
		void FixSvr();
		void CleanSvr();
		
		vector<Svr_t*> mSvr;
		map<int, Svr_t*> mSvrById;
		map<int, vector<Svr_t*> > mSvrByGid;
		map<string, vector<Svr_t*> > mSvrByName;
		map<string, vector<Svr_t*> > mSvrByGXid;

		TiXmlDocument* mDoc;
};

#endif
