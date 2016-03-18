
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
		void Final();
		
		void GetBaseConf();
		void GetSvrConf();
		void GetQosConf();

		bool IsModTime();
		int SetModTime();
		int GetModSvr(SvrNet_t* pBuffer);

		/*
		int GetSvrAll(SvrNet_t* pBuffer);
		int ReloadSvr(SvrNet_t* pBuffer);
		*/
	
	protected:
		SvrQos *mSvrQos;
		TiXmlDocument* mDoc;

		char mSvrConfFile[255];
		char mQosConfFile[255];
		char mBaseConfFile[255];
		time_t mMtime;	//svr.xml修改时间

		/*
		void SvrRebuild();
		void DelContainer();
		vector<Svr_t*>::iterator GetItFromV(Svr_t* pSvr);
		vector<Svr_t*>::iterator GetItById(int iId);
		
		bool IsChangeSvr(const SvrNet_t* pR1, const SvrNet_t* pR2);

		int GetAllSvrByGXid(SvrNet_t* pBuffer, int iGid, int iXid);
		int GetAllSvrByGid(SvrNet_t* pBuffer, int iGid);
		void SetGXDirty(Svr_t* stSvr, int iDirty = 1);

		int CalcWeight(Svr_t* stSvr);
		short CalcPre(Svr_t* stSvr);
		short CalcOverLoad(Svr_t* stSvr);
		short CalcShutdown(Svr_t* stSvr);

		int GcdWeight(vector<Svr_t*> vSvr, int n);
		void ReleaseConn(Svr_t* stSvr);
		int GetSumConn(Svr_t* stSvr);
		*/
	
		//vector<Svr_t*> mSvr;	//统计表
		//map<string, vector<Svr_t*> > mSvrByGXid;	//路由表
		//map<string, StatcsGXid_t*> mStatcsGXid;		//WRR统计表

		//阈值配置
		//SvrReqCfg_t mSvrReqCfg;
		//SvrListCfg_t mSvrListCfg;
		//SvrDownCfg_t mSvrDownCfg;
};

#endif
