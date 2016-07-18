
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
		RouterConfig();
		virtual ~RouterConfig();
		
		void GetBaseConf();
		void GetSvrConf();
		void GetQosConf();

		bool IsModTime();
		int SetModTime();
		int GetModSvr(SvrNet_t* pBuffer);
		
		SvrQos *Qos() { return mSvrQos; }
	public:
		char mIPAddr[MAX_IP_LEN] {'\0'};
		unsigned int mPort {0};
		unsigned int mBacklog {LISTEN_BACKLOG};
		unsigned int mWorkers {1};

	protected:
		time_t mMtime {0};	//svr.xml修改时间
		
		SvrQos *mSvrQos {NULL};
		TiXmlDocument *mDoc {NULL};
		wMemPool *mMemPool {NULL};

		string mSvrConfFile = SVR_XML;
		string mQosConfFile = QOS_XML;
		string mBaseConfFile = CONF_XML;
};

#endif
