
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _ROUTER_CONFIG_H_
#define _ROUTER_CONFIG_H_

#include <map>
#include <vector>
#include <algorithm>

#include "wType.h"
#include "wMisc.h"
#include "wLog.h"
#include "Svr.h"
#include "wConfig.h"

#define CONF_XML "../config/conf.xml"
#define SVR_XML "../config/svr.xml"

class RouterConfig: public wConfig<RouterConfig>
{
	public:
		char mIPAddr[MAX_IP_LEN];
		unsigned int mPort;
		unsigned int mBacklog;
		unsigned int mWorkers;

	public:
		RouterConfig();

		virtual ~RouterConfig();

		//初始化
		void Initialize();
		
		void Final();
		
		/**
		 * 解析配置
		 */
		void GetBaseConf();
		
		/**
		 *  解析Svr配置
		 */
		void GetSvrConf();
		
		bool IsModTime();
		int SetModTime();
		int GetModSvr(SvrNet_t* pBuffer);
		int GetSvrAll(SvrNet_t* pBuffer);
		int ReloadSvr(SvrNet_t* pBuffer);
		
	protected:
		bool IsChangeSvr(const SvrNet_t* pR1, const SvrNet_t* pR2);
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
