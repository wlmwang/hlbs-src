
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _SVR_QOS_H_
#define _SVR_QOS_H_

#include <vector>
#include <map>
#include <list>
#include <algorithm>
#include <cmath>

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

		int SaveNode(struct SvrNet_t& stSvr);

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
		int mRateWeight;	//成功率因子	1~100000 默认1
		int mDelayWeight;	//时延因子		1~100000 默认1
	    int mRebuildTm;		//重建时间间隔 默认为3s
	    int mReqTimeout;	//请求超时时间 默认为500ms

	    float mAvgErrRate;		//错误平均值（过载时）
	    bool mAllReqMin;		//所有节点都过载？

	    SvrReqCfg_t	 mReqCfg;	//访问量控制
	    SvrListCfg_t mListCfg;	//并发量控制
		SvrDownCfg_t mDownCfg;	//宕机控制
		
		map<struct SvrNet_t, struct SvrStat_t*>	mMapReqSvr;		//节点信息。路由-统计，一对一
		map<struct SvrKind_t, multimap<float, SvrNode_t>* > mRouteTable;	//路由信息。种类-节点，一对多
		map<struct SvrKind_t, list<struct SvrNode_t>* > mErrTable;		//宕机路由表
	    
	    //QOSTMCFG	_qos_tm_cfg;	//OS 每个时间节点(0-19)统计数据
};

#endif