
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _AGENT_CONFIG_H_
#define _AGENT_CONFIG_H_

#include <vector>
#include <map>

#include "wCore.h"
#include "wConfig.h"
#include "wMisc.h"
#include "wLog.h"
#include "wSingleton.h"
#include "SvrCmd.h"

#define MAX_ROUTER_NUM 32

#define CONF_XML "../config/conf.xml"
#define ROUTER_XML "../config/router.xml"

class AgentConfig: public wConfig<AgentConfig>
{
	public:
		//router配置，server.xml
		struct RouterConf_t
		{
			char mIPAddr[MAX_IP_LEN];
			unsigned int mPort;
			short mDisabled;
			RouterConf_t()
			{
				memset(mIPAddr, 0, sizeof(mIPAddr));
				mPort = 0;
				mDisabled = 0;
			}
		};
		RouterConf_t mRouterConf[MAX_ROUTER_NUM];

		char mIPAddr[MAX_IP_LEN];
		unsigned int mPort;
		unsigned int mBacklog;
		unsigned int mWorkers;

		RouterConf_t* GetOneRouterConf()
		{
			for(int i = 0; i < MAX_ROUTER_NUM ; i++)
			{
				if (mRouterConf[i].mPort != 0 && mRouterConf[i].mDisabled == 0 && strlen(mRouterConf[i].mIPAddr) != 0)
				{
					return &mRouterConf[i];
				}
			}
			return NULL;
		}
		AgentConfig();
		void Initialize();
		virtual ~AgentConfig();
		void Final();

		void GetBaseConf();
		void GetRouterConf();

		//初始化svr
		int InitSvr(SvrNet_t *pSvr, int iLen = 0);
		int ReloadSvr(SvrNet_t *pSvr, int iLen = 0);
		int SyncSvr(SvrNet_t *pSvr, int iLen = 0);
		
		//CGI获取所有svr
		int GetSvrAll(SvrNet_t* pBuffer);
		//CGI获取一个路由
		int GetSvrByGXid(SvrNet_t* pBuffer, int iGid, int iXid);
		
		//接受上报数据
		void ReportSvr(SvrReportReqId_t *pReportSvr);
		void Statistics();
		
		int GXidWRRSvr(SvrNet_t* pBuffer, string sKey, vector<Svr_t*> vSvr);

	protected:
		struct StatcsGXid_t
		{
			int mSumConn;//该类目下总共被分配次数

			int mIdx;	//当前分配到索引号
			int mCur;	//当前weight。初始化为最大值
			int mGcd;	//weight最大公约数
			StatcsGXid_t()
			{
				mIdx = -1;
				mCur = 0;
				mGcd = 0;
				mSumConn = 0;
			}
		};

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

		TiXmlDocument* mDoc;
		
		vector<Svr_t*> mSvr;
		map<string, vector<Svr_t*> > mSvrByGXid;
		map<string, StatcsGXid_t*> mStatcsGXid;

		//阈值配置
		SvrReqCfg_t mSvrReqCfg;
		SvrListCfg_t mSvrListCfg;
		SvrDownCfg_t mSvrDownCfg;
};

#endif
