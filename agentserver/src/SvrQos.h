
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _SVR_QOS_H_
#define _SVR_QOS_H_

#include <vector>
#include <map>
#include <list>
#include <algorithm>
#include <iterator>
#include <cmath>

#include "wCore.h"
#include "wMisc.h"
#include "wLog.h"
#include "wSingleton.h"
#include "SvrCmd.h"
#include "wMutex.h"
#include "DetectThread.h"

class AgentConfig;
class SvrQos : public wSingleton<SvrQos>
{
	friend class AgentConfig;
	public:
		SvrQos();
		~SvrQos();
		void Initialize();
		
		int CleanNode();
		int SaveNode(struct SvrNet_t& stSvr);
		int ModNode(struct SvrNet_t& stSvr);
		int QueryNode(struct SvrNet_t& stSvr);
		int CallerNode(struct SvrCaller_t& stCaller);
		int NotifyNode(struct SvrNet_t& stSvr);
		int GetSvrAll(struct SvrNet_t* pBuffer);

		bool IsExistNode(struct SvrNet_t& stSvr);
		bool IsVerChange(struct SvrNet_t& stSvr);
	private:
		int GetRouteNode(struct SvrNet_t& stSvr);
		int RouteCheck(struct SvrStat_t* pSvrStat, struct SvrNet_t& stNode, double iFirstReq, bool bFirstRt);
		int RouteNodeRebuild(struct SvrNet_t &stSvr, struct SvrStat_t* pSvrStat);
		int LoadStatCfg(struct SvrNet_t& stSvr, struct SvrStat_t* pSvrStat);
		int AddRouteNode(struct SvrNet_t& stSvr, struct SvrStat_t* pSvrStat);
		int AllocNode(struct SvrNet_t& stSvr);
		int DelNode(struct SvrNet_t& stSvr);
		int ModRouteNode(struct SvrNet_t& stSvr);
		int DelRouteNode(struct SvrNet_t& stSvr);
		int GetAddCount(struct SvrStat_t* pSvrStat, int iReqCount);
		int ReqRebuild(struct SvrNet_t &stSvr, struct SvrStat_t* pSvrStat);
		int ListRebuild(struct SvrNet_t &stSvr, struct SvrStat_t* pSvrStat);
		int RebuildRoute(struct SvrKind_t& stItem, int bForce = false);
		int AddErrRoute(struct SvrKind_t& stItem, struct SvrNode_t& stNode);
		int RebuildErrRoute(struct SvrKind_t& stItem, multimap<float, struct SvrNode_t>* pSvrNode, float iPri = 1, float fLowOkRate = 1, unsigned int iBigDelay = 1);
		map<struct SvrNet_t, struct SvrStat_t*>::iterator SearchNode(struct SvrNet_t& stSvr);

		/** for test */
		void LogAllNode();

	protected:
		DetectThread *mDetectThread;

		int mRateWeight;	//成功率因子	1~100000 默认1
		int mDelayWeight;	//时延因子		1~100000 默认1
	    int mRebuildTm;		//重建时间间隔 默认为3s
	    int mReqTimeout;	//请求超时时间 默认为500ms

	    float mAvgErrRate;		//所有节点过载平均错误率
	    bool mAllReqMin;		//所有节点都过载

	    SvrReqCfg_t	 mReqCfg;	//访问量控制
	    SvrListCfg_t mListCfg;	//并发量控制
		SvrDownCfg_t mDownCfg;	//宕机控制
		
		int mPreRoute;			//是否开启预取缓存功能。开启后可以预先将某个gid, xid对应的host,port加载到内存中，以节省访问资源。
		map<struct SvrNet_t, struct SvrStat_t*>	mMapReqSvr;		//节点信息。路由-统计，一对一
		map<struct SvrKind_t, multimap<float, SvrNode_t>* > mRouteTable;	//路由信息。种类-节点，一对多
		map<struct SvrKind_t, list<struct SvrNode_t>* > mErrTable;		//宕机路由表
};

#endif