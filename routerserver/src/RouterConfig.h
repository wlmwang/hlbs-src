
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _ROUTER_CONFIG_H_
#define _ROUTER_CONFIG_H_

#include <map>
#include <vector>
#include <algorithm>

#include "wCore.h"
#include "wMisc.h"
#include "wLog.h"
#include "wConfig.h"
#include "SvrQos.h"

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
		void Initialize();
		void Final();
		
		void GetBaseConf();
		void GetSvrConf();

		/*
		bool IsModTime();
		int SetModTime();
		int GetModSvr(SvrNet_t* pBuffer);
		int GetSvrAll(SvrNet_t* pBuffer);
		int ReloadSvr(SvrNet_t* pBuffer);
		*/
	
	protected:
		TiXmlDocument* mDoc;
		char mSvrConfFile[255];
		char mBaseConfFile[255];
		time_t mMtime;	//svr.xml修改时间

		SvrQos *mSvrQos;

		/*
		bool IsChangeSvr(const SvrNet_t* pR1, const SvrNet_t* pR2);
		void SvrRebuild();
		void DelContainer();
		vector<Svr_t*>::iterator GetItFromV(Svr_t* pSvr);
		
		vector<Svr_t*> mSvr;
		map<int, vector<Svr_t*> > mSvrByGid;
		map<string, vector<Svr_t*> > mSvrByGXid;
		*/
};

#endif
