
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
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
#include "wMemPool.h"

#define CONF_XML "../config/conf.xml"
#define SVR_XML "../config/svr.xml"
#define QOS_XML "../config/qos.xml"

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
		
		void GetBaseConf();
		void GetSvrConf();
		void GetQosConf();

		bool IsModTime();
		int SetModTime();
		int GetModSvr(SvrNet_t* pBuffer);
		
		SvrQos *Qos() { return mSvrQos; }
		
	protected:
		SvrQos *mSvrQos;
		TiXmlDocument* mDoc;
		wMemPool *mMemPool;
		
		char mSvrConfFile[255];
		char mQosConfFile[255];
		char mBaseConfFile[255];
		time_t mMtime;	//svr.xml修改时间
};

#endif
