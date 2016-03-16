
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _SVR_QOS_H_
#define _SVR_QOS_H_

#include <vector>
#include <map>
#include <list>

#include "wCore.h"
#include "wMisc.h"
#include "wLog.h"
#include "wSingleton.h"
#include "SvrCmd.h"

class SvrQos : public wSingleton<SvrQos>
{
	public:
		SvrQos();
		~SvrQos();

		int AddNode(struct SvrNet_t stSvr);

		/*
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
		*/

	protected:
	    int 		mRebuildTm;	//QOS重建时间间隔  >3
	    SvrReqCfg_t	mReqCfg;	//QOS访问量控制的默认配置
	    SvrListCfg_t mListCfg;	//QOS并发量控制的默认配置

		map<struct SvrNet_t, struct SvrStat_t*>	mMapReqSvr;
		
		map<struct SvrKind_t, multimap<float, SvrNode_t>* > mRouteTable;
		map<struct SvrKind_t, list<SvrNode_t>* > mErrTable;
	    
	    //QOSTMCFG	_qos_tm_cfg;	//OS 每个时间节点(0-19)统计数据
		
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