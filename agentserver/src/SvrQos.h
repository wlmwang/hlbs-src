
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _SVR_QOS_H_
#define _SVR_QOS_H_

#include <map>
#include <list>
#include "wCore.h"
#include "wStatus.h"
#include "wNoncopyable.h"
#include "SvrCmd.h"

using namespace hnet;

class AgentConfig;

class SvrQos : private wNoncopyable {
public:
	~SvrQos();
	SvrQos() : mRateWeight(7), mDelayWeight(1), mRebuildTm(60), mReqTimeout(500),
			mPreRoute(1), mAllReqMin(false), mAvgErrRate(0.0) { }

	// 指定节点是否存在
	bool IsExistNode(const struct SvrNet_t& svr);
	// 节点是否变化。节点不存在时，接口认为“版本变化”（新配置始终下发，旧配置检测到version变化才下发）
	bool IsVerChange(const struct SvrNet_t& svr);

	// 获取 最优节点
	const wStatus& QueryNode(struct SvrNet_t& svr);
	// 上报 调用结果
	const wStatus& CallerNode(const struct SvrCaller_t& caller);
	// 获取 所有节点
	const wStatus& GetNodeAll(struct SvrNet_t buf[], int32_t* num);
	// 保存 节点信息
	const wStatus& SaveNode(const struct SvrNet_t& svr);
	// 添加 新节点&&路由
	const wStatus& AddNode(const struct SvrNet_t& svr);
	// 修改 节点&&路由 权重（权重为0删除节点）
	const wStatus& ModifyNode(const struct SvrNet_t& svr);
	// 删除 节点&&路由
	const wStatus& DeleteNode(const struct SvrNet_t& svr);
	// 清除 所有路由信息
	const wStatus& CleanNode();

protected:
	friend class AgentConfig;

	// 门限扩张值
	int32_t GetAddCount(const struct SvrStat_t* stat, int32_t reqCount);
	// 重建该类路由，并清理相关统计
	const wStatus& RebuildRoute(struct SvrKind_t& kind, bool force = false);
	// 路由节点重建
	const wStatus& RouteNodeRebuild(const struct SvrNet_t &svr, struct SvrStat_t* stat);
	// 节点访问量控制重建
	const wStatus& ReqRebuild(const struct SvrNet_t &svr, struct SvrStat_t* stat);
	//  重建宕机路由（故障恢复，恢复后放入multiNode指针中）
	const wStatus& RebuildErrRoute(struct SvrKind_t& kind, MultiMapNode_t* multiNode, float maxLoad = 1.0, float lowOkRate = 1.0, uint32_t bigDelay = 1);

	// 单次分配路由检查，如果有路由分配产生，则更新相关统计计数
	int RouteCheck(struct SvrStat_t* stat, struct SvrNet_t& svr, double firstLoad, bool firstSvr);
	// 单次获取路由
	const wStatus& GetRouteNode(struct SvrNet_t& svr);
	// 添加宕机路由
	const wStatus& AddErrRoute(struct SvrKind_t& kind, struct SvrNode_t& node);
	// 添加新路由
	const wStatus& AddRouteNode(const struct SvrNet_t& svr, struct SvrStat_t* stat);
	// 删除路由节点
	const wStatus& DeleteRouteNode(const struct SvrNet_t& svr);
	// 修改路由节点
	const wStatus& ModifyRouteNode(const struct SvrNet_t& svr);
	// 加载阈值配置
	const wStatus& LoadStatCfg(const struct SvrNet_t& svr, struct SvrStat_t* stat);

	struct SvrReqCfg_t	mReqCfg;	// 访问量控制
	struct SvrDownCfg_t mDownCfg;	// 宕机控制

	int mRateWeight;	// 成功率因子 1~100000
	int mDelayWeight;	// 时延因子 1~100000
	int mRebuildTm;		// 重建时间间隔 默认为60s
	int mReqTimeout;	// 请求超时时间 默认为500ms
	bool mAllReqMin;	// 所有节点都过载
	float mAvgErrRate;	// 错误平均值，过载时

	std::map<struct SvrNet_t, struct SvrStat_t*> mMapReqSvr;	// 节点信息。1:1，节点-统计
	std::map<struct SvrKind_t, std::multimap<float, struct SvrNode_t>* > mRouteTable;	// 路由信息。1:n，种类-节点
	std::map<struct SvrKind_t, std::list<struct SvrNode_t>* > mErrTable;	// 宕机路由表，1:n，种类-节点

	typedef std::map<struct SvrNet_t, struct SvrStat_t*> MapSvr_t;
	typedef std::map<struct SvrNet_t, struct SvrStat_t*>::iterator MapSvrIt_t;

	typedef std::map<struct SvrKind_t, std::multimap<float, struct SvrNode_t>* > MapKind_t;
	typedef std::map<struct SvrKind_t, std::multimap<float, struct SvrNode_t>* >::iterator MapKindIt_t;
	typedef std::multimap<float, struct SvrNode_t> MultiMapNode_t;
	typedef std::multimap<float, struct SvrNode_t>::iterator MultiMapNodeIt_t;
	typedef std::multimap<float, struct SvrNode_t>::reverse_iterator MultiMapNodeRIt_t;

    typedef std::map<struct SvrKind_t, std::list<struct SvrNode_t>* > MapNode_t;
    typedef std::map<struct SvrKind_t, std::list<struct SvrNode_t>* >::iterator MapNodeIt_t;
    typedef std::list<struct SvrNode_t> ListNode_t;
    typedef std::list<struct SvrNode_t>::iterator ListNodeIt_t;

	wStatus mStatus;
};

#endif
