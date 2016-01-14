
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _AGENT_CONFIG_H_
#define _AGENT_CONFIG_H_

#include <string.h>
#include <vector>
#include <map>

#include "wConfig.h"
#include "wType.h"
#include "wLog.h"
#include "wSingleton.h"
#include "SvrCommand.h"

/**
 * 配置文件读取的数据结构
 */
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
		RouterConf_t mRouterConf[32];

		char mIPAddr[MAX_IP_LEN];
		unsigned int mPort;
		unsigned int mBacklog;
		unsigned int mWorkers;

		RouterConf_t* GetOneRouterConf()
		{
			for(int i = 0; i < 32 ; i++)
			{
				if (mRouterConf[i].mPort != 0 && mRouterConf[i].mDisabled == 0 && strlen(mRouterConf[i].mIPAddr) != 0)
				{
					return &mRouterConf[i];
				}
				return NULL;
			}
		}
		
		//初始化
		void Initialize();

		AgentConfig()
		{
            Initialize();
		}

		void Final();
		virtual ~AgentConfig() 
		{
			Final();
			SAFE_DELETE(mDoc);
		}

		void ParseBaseConfig();
		void ParseRouterConfig();

		int InitSvr(SvrNet_t *pSvr, int iLen = 0);
		int ReloadSvr(SvrNet_t *pSvr, int iLen = 0);
		int SyncSvr(SvrNet_t *pSvr, int iLen = 0);
		
		int GetSvrAll(SvrNet_t* pBuffer);
		int GetSvrByGid(SvrNet_t* pBuffer, int iGid);
		int GetSvrByGXid(SvrNet_t* pBuffer, int iGid, int iXid);
		
		void ReportSvr(SvrReportReqId_t *pReportSvr);	//上报结果
		void Statistics();
		
	protected:
		void FixContainer();
		void DelContainer();
		vector<Svr_t*>::iterator GetItFromV(Svr_t* pSvr);
		vector<Svr_t*>::iterator GetItById(int iId);
		
		bool IsChangeSvr(const Svr_t* pR1, const Svr_t* pR2);

		int GetAllSvrByGXid(SvrNet_t* pBuffer, int iGid, int iXid);
		int GetAllSvrByGid(SvrNet_t* pBuffer, int iGid);
		void SetGXDirty(Svr_t* stSvr, int iDirty = 1);

		int CalcWeight(Svr_t* stSvr);
		short CalcPre(Svr_t* stSvr);
		short CalcOverLoad(Svr_t* stSvr);
		
		struct RunWRR_t
		{
			int mIndex;
			int mWeight;	//初始化最大值
			int mWeightGcd;	//公约数（通用wrr算法）
			int mWeightAva;	//平均值
			RunWRR_t()
			{
				mIndex = -1;
				mWeight = 0;
				mWeightGcd = 0;
				mWeightAva = 0;
			}
		};
		vector<Svr_t*> mSvr;
		map<int, vector<Svr_t*> > mSvrByGid;
		map<string, vector<Svr_t*> > mSvrByGXid;

		map<int, RunWRR_t*> mRunWrrGid;
		map<string, RunWRR_t*> mRunWrrGXid;

		TiXmlDocument* mDoc;
};

#endif
