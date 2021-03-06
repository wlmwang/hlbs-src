
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
#include "wMutex.h"
#include "SvrCmd.h"

using namespace hnet;

class AgentConfig;
class DetectThread;

class SvrQos : private wNoncopyable {
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

public:
    SvrQos();
	~SvrQos();

	// 获取 最优节点
	const wStatus& QueryNode(struct SvrNet_t& svr);
	// 调用数上报
	const wStatus& NtyNodeSvr(const struct SvrNet_t& svr);
	// 上报 调用结果
	const wStatus& CallerNode(const struct SvrCaller_t& caller);
	// 获取 所有节点
	const wStatus& GetNodeAll(struct SvrNet_t buf[], int32_t* num, int32_t start, int32_t size);
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

	int StartDetectThread();

	// 指定节点是否存在
	bool IsExistNode(const struct SvrNet_t& svr);

	inline int& Idc() { return mIdc;}

protected:
	friend class AgentConfig;

	// 上报 调用结果
	int ReportNode(const struct SvrCaller_t& caller);

	// 门限扩张值
	int32_t GetAddCount(const struct SvrStat_t* stat, int32_t reqCount);

	// 单次分配路由检查，如果有路由分配产生，则更新相关统计计数
	int RouteCheck(struct SvrStat_t* stat, struct SvrNet_t& svr, double firstLoad, bool firstSvr, pid_t pid = 0);

	// 重建该类路由，并清理相关统计
	void RebuildRoute(struct SvrKind_t& kind, bool force = false);

	// 使用已有统计数据计算节点负载均衡各项数据：门限控制、计算成功率、平均延时值
	void RouteNodeRebuild(const struct SvrNet_t &svr, struct SvrStat_t* stat);

	// 重建节点访问量控制
	void ReqRebuild(const struct SvrNet_t &svr, struct SvrStat_t* stat);

	//  重建宕机路由（故障恢复，恢复后放入multiNode指针中）
	void RebuildErrRoute(struct SvrKind_t& kind, MultiMapNode_t* multiNode, float maxLoad = 1.0, float lowOkRate = 1.0, uint32_t bigDelay = 1);

	// 单次获取路由
	int GetRouteNode(struct SvrNet_t& svr);

	// 添加宕机路由
	int AddErrRoute(struct SvrKind_t& kind, struct SvrNode_t& node);

	// 添加新路由
	int AddRouteNode(const struct SvrNet_t& svr, struct SvrStat_t* stat);

	// 删除路由节点
	int DeleteRouteNode(const struct SvrNet_t& svr);

	// 修改路由节点
	int ModifyRouteNode(const struct SvrNet_t& svr);

	// 加载阈值配置
	void LoadStatCfg(const struct SvrNet_t& svr, struct SvrStat_t* stat);

	struct SvrReqCfg_t	mReqCfg;	// 访问量控制
	struct SvrDownCfg_t mDownCfg;	// 宕机控制

	int mIdc;
	int mRateWeight;	// 成功率因子 1~100000
	int mDelayWeight;	// 时延因子 1~100000
	int mRebuildTm;		// 重建时间间隔 默认为60s
	int mReqTimeout;	// 请求超时时间 默认为500ms
	bool mAllReqMin;	// 所有节点都过载
	float mAvgErrRate;	// 错误平均值，过载时

	MapSvr_t mMapReqSvr;	// 节点信息。1:1，节点-统计
	MapKind_t mRouteTable;	// 路由信息。1:n，种类-节点
	MapNode_t mErrTable;	// 宕机路由表，1:n，种类-节点

	wMutex mMutex;	// TcpTask|ClientTask更新路由锁
    DetectThread* mDetectThread;

	wStatus mStatus;
};

#endif
