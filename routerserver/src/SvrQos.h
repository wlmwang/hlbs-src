
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
#include <cmath>
#include "wCore.h"
#include "wStatus.h"
#include "wNoncopyable.h"
#include "SvrCmd.h"

using namespace hnet;

class SvrQos : private wNoncopyable {
public:
	SvrQos() mRateWeight(7), mDelayWeight(1), mRebuildTm(60), mReqTimeout(500), mAvgErrRate(0.0), mAllReqMin(false), mPreRoute(1) { };
	~SvrQos();

	int SaveNode(struct SvrNet_t& stSvr);
	int ModNode(struct SvrNet_t& stSvr);
	int AllocNode(struct SvrNet_t& stSvr);
	int QueryNode(struct SvrNet_t& stSvr);
	int DelNode(const struct SvrNet_t& stSvr);
	int NotifyNode(const struct SvrNet_t& stSvr);
	int CallerNode(const struct SvrCaller_t& stCaller);
	int CleanNode();

	bool IsExistNode(const struct SvrNet_t& stSvr);
	bool IsVerChange(const struct SvrNet_t& stSvr);
	std::map<struct SvrNet_t, struct SvrStat_t*>::iterator SearchNode(const struct SvrNet_t& stSvr);

	int GetSvrAll(struct SvrNet_t vBuffer[]);

protected:
	int LoadStatCfg(struct SvrNet_t& stSvr, struct SvrStat_t* pSvrStat);
	int AddRouteNode(const struct SvrNet_t& stSvr, struct SvrStat_t* pSvrStat);
	int GetRouteNode(struct SvrNet_t& stSvr);
	int RouteCheck(struct SvrStat_t* pSvrStat, struct SvrNet_t& stNode, double iFirstReq, bool bFirstRt);
	int RouteNodeRebuild(const struct SvrNet_t &stSvr, struct SvrStat_t* pSvrStat);
	int ModRouteNode(struct SvrNet_t& stSvr);
	int DelRouteNode(const struct SvrNet_t& stSvr);
	int GetAddCount(struct SvrStat_t* pSvrStat, int iReqCount);
	int ReqRebuild(const struct SvrNet_t &stSvr, struct SvrStat_t* pSvrStat);
	int ListRebuild(const struct SvrNet_t &stSvr, struct SvrStat_t* pSvrStat);
	int RebuildRoute(struct SvrKind_t& stItem, int bForce = false);
	int AddErrRoute(struct SvrKind_t& stItem, struct SvrNode_t& stNode);
	int RebuildErrRoute(struct SvrKind_t& stItem, multimap<float, struct SvrNode_t>* pSvrNode);

	/** for test */
	void LogAllNode();

	struct SvrReqCfg_t	mReqCfg;	// 访问量控制
	struct SvrDownCfg_t mDownCfg;	// 宕机控制

	int mRateWeight;	// 成功率因子 1~100000
	int mDelayWeight;	// 时延因子 1~100000
	int mRebuildTm;		// 重建时间间隔 默认为60s
	int mReqTimeout;	// 请求超时时间 默认为500ms
	int mPreRoute;		// 是否开启预取缓存功能。开启后可以预先将某个gid, xid对应的host,port加载到内存中，以节省访问资源。
	bool mAllReqMin;	// 所有节点都过载
	float mAvgErrRate;	// 错误平均值，过载时

	std::map<struct SvrNet_t, struct SvrStat_t*> mMapReqSvr;	// 节点信息。路由-统计，一对一
	std::map<struct SvrKind_t, std::multimap<float, struct SvrNode_t>* > mRouteTable;	// 路由信息。种类-节点，一对多
	std::map<struct SvrKind_t, std::list<struct SvrNode_t>* > mErrTable;	// 宕机路由表
};

#endif
