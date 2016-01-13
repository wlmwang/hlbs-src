
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

		/**
		 * 解析配置
		 */		
		void ParseBaseConfig();
		void ParseRouterConfig();

		int GetSvrAll(SvrNet_t* pBuffer);
		int GetSvrById(SvrNet_t* pBuffer, int iId);

		int InitSvr(SvrNet_t *pBuffer, int iLen = 0);
		int ReloadSvr(SvrNet_t *pBuffer, int iLen = 0);
		int SyncSvr(SvrNet_t *pBuffer, int iLen = 0);

		int GetSvrByName(SvrNet_t* pBuffer, string sName, int iNum = 0);
		int GetSvrByGid(SvrNet_t* pBuffer, int iGid, int iNum = 0);
		int GetSvrByGXid(SvrNet_t* pBuffer, int iGid, int iXid, int iNum = 0);
		
		BYTE SetSvrAttr(WORD iId, BYTE iDisabled, WORD iWeight, WORD iTimeline, WORD iConnTime, WORD iTasks, WORD iSuggest);

		void ReportSvr(SvrReportReqId_t *pReportSvr);	//上报结果
		void Statistics();

	protected:
		BYTE DisabledSvr(WORD iId);
		BYTE SetSvrWeight(WORD iId, WORD iWeight);

		void FixContainer();
		void DelContainer();

		vector<Svr_t*>::iterator GetItById(int iId);
		void SetGXDirty(Svr_t* stSvr, int iDirty = 1);

		float CalcWeight(Svr_t* stSvr);
		int CalcPre(Svr_t* stSvr);
		int CalcOverLoad(Svr_t* stSvr);
		
		vector<Svr_t*> mSvr;
		map<int, Svr_t*> mSvrById;
		map<int, vector<Svr_t*> > mSvrByGid;
		map<string, vector<Svr_t*> > mSvrByName;
		map<string, vector<Svr_t*> > mSvrByGXid;

		TiXmlDocument* mDoc;
};

#endif
