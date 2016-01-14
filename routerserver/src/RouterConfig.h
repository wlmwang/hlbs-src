
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _ROUTER_CONFIG_H_
#define _ROUTER_CONFIG_H_

#include <unistd.h>
#include <string.h>
#include <string>
#include <map>
#include <vector>
#include <sys/stat.h>

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
			SAFE_DELETE(mDoc);
		}

		//初始化
		void Initialize()
		{
			memset(mIPAddr, 0, sizeof(mIPAddr));
			mPort = 0;
			mBacklog = 1024;
			mWorkers = 1;
			mDoc = new TiXmlDocument();
			memcpy(mBaseConfFile, "../config/conf.xml", sizeof("../config/conf.xml"));
			memcpy(mSvrConfFile, "../config/svr.xml", sizeof("../config/svr.xml"));
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
		
		bool IsModTime();
		int SetModTime();
		int GetModSvr(SvrNet_t* pBuffer);
		int GetSvrAll(SvrNet_t* pBuffer);
		int ReloadSvr(SvrNet_t* pBuffer);
	protected:
		bool IsChangeSvr(const Svr_t* pR1, const Svr_t* pR2);
		void FixContainer();
		void DelContainer();
		vector<Svr_t*>::iterator GetItFromV(Svr_t* pSvr);
		
		vector<Svr_t*> mSvr;
		map<int, vector<Svr_t*> > mSvrByGid;
		map<string, vector<Svr_t*> > mSvrByGXid;

		time_t mMtime;	//svr.xml 修改时间
		TiXmlDocument* mDoc;

		char mSvrConfFile[32];
		char mBaseConfFile[32];
};

#endif
