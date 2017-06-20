
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include <vector>
#include <algorithm>
#include <cmath>
#include "wLogger.h"
#include "SvrQos.h"
#include "Detect.h"
#include "DetectThread.h"

SvrQos::SvrQos(): mIdc(0), mRateWeight(7), mDelayWeight(1), mRebuildTm(60), mReqTimeout(500),mAllReqMin(false), mAvgErrRate(0.0) {
	SAFE_NEW(DetectThread(mDownCfg.mProbeInterval), mDetectThread);
}

SvrQos::~SvrQos() {
	SAFE_DELETE(mDetectThread);
	CleanNode();
}

bool SvrQos::IsExistNode(const struct SvrNet_t& svr) {
	mMutex.Lock();
	MapSvrIt_t mapReqIt = mMapReqSvr.find(svr);
	MapSvrIt_t mapEndIt = mMapReqSvr.end();
	mMutex.Unlock();
	if (mapReqIt == mapEndIt) {
		return false;
	}
	return true;
}

const wStatus& SvrQos::StartDetectThread() {
	// 探测线程，宕机拉起
	if (!(mStatus = mDetectThread->StartThread()).Ok()) {
		return mStatus;
	}
	return mStatus;
}

const wStatus& SvrQos::SaveNode(const struct SvrNet_t& svr) {
	LOG_DEBUG(kSvrLog, "SvrQos::SaveNode save SvrNet_t start, GID(%d),XID(%d),HOST(%s),PORT(%d),WEIGHT(%d)",
			svr.mGid, svr.mXid, svr.mHost, svr.mPort, svr.mWeight);

	if (IsExistNode(svr)) {
		return ModifyNode(svr);
	}
	return AddNode(svr);
}

const wStatus& SvrQos::AddNode(const struct SvrNet_t& svr) {
	if (svr.mWeight <= 0) {
		LOG_ERROR(kSvrLog, "SvrQos::AddNode add SvrNet_t failed(weight<=0), GID(%d),XID(%d),HOST(%s),PORT(%d),WEIGHT(%d)",
				svr.mGid, svr.mXid, svr.mHost, svr.mPort, svr.mWeight);

		return mStatus = wStatus::IOError("SvrQos::AddNode failed, the SvrNet_t(weight<=0) will be ignore", "");
	}

	struct SvrStat_t* stat;
	SAFE_NEW(SvrStat_t, stat);

	LoadStatCfg(svr, stat);

	LOG_DEBUG(kSvrLog, "SvrQos::AddNode add SvrNet_t success, GID(%d),XID(%d),HOST(%s),PORT(%d),WEIGHT(%d)",
			svr.mGid, svr.mXid, svr.mHost, svr.mPort, svr.mWeight);

	mMutex.Lock();
	stat->mReqCfg.mRebuildTm = mRebuildTm;	// 重建绝对时间
	misc::GetTimeofday(&stat->mInfo.mBuildTm);
	mMapReqSvr.insert(std::make_pair(svr, stat));
	if (AddRouteNode(svr, stat) == -1) {
		mStatus = wStatus::IOError("SvrQos::AddNode add SvrNet_t failed", "");
	}
	mMutex.Unlock();

	return mStatus;
}

const wStatus& SvrQos::ModifyNode(const struct SvrNet_t& svr) {
	LOG_DEBUG(kSvrLog, "SvrQos::ModifyNode modify SvrNet_t start, GID(%d),XID(%d),HOST(%s),PORT(%d),WEIGHT(%d)",
			svr.mGid, svr.mXid, svr.mHost, svr.mPort, svr.mWeight);

	if (svr.mWeight < 0) {
		LOG_ERROR(kSvrLog, "SvrQos::ModifyNode modify SvrNet_t failed(weight < 0), GID(%d),XID(%d),HOST(%s),PORT(%d),WEIGHT(%d)",
				svr.mGid, svr.mXid, svr.mHost, svr.mPort, svr.mWeight);

		mStatus = wStatus::IOError("SvrQos::ModifyNode failed, the SvrNet_t(weight < 0) will be ignore", "");
	} else if (svr.mWeight == 0) {
		// 权重为0删除节点
		mStatus = DeleteNode(svr);
	} else {	// 修改weight
		mMutex.Lock();
		MapSvrIt_t mapReqIt = mMapReqSvr.find(svr);
		struct SvrNet_t& oldsvr = const_cast<struct SvrNet_t&>(mapReqIt->first);
		oldsvr = svr;
		if (ModifyRouteNode(svr) == -1) {
			mStatus = wStatus::IOError("SvrQos::ModifyNode ModifyRouteNode() failed, the SvrNet_t(weight > 0) update failed", "");
		}
		mMutex.Unlock();
	}

	return mStatus;
}

const wStatus& SvrQos::DeleteNode(const struct SvrNet_t& svr) {
	mMutex.Lock();
	MapSvrIt_t mapReqIt = mMapReqSvr.find(svr);
	MapSvrIt_t mapEndIt = mMapReqSvr.end();
	mMutex.Unlock();
	if (mapReqIt == mapEndIt) {
		LOG_ERROR(kSvrLog, "SvrQos::DeleteNode delete SvrNet_t failed(cannot find the SvrNet_t), GID(%d),XID(%d),HOST(%s),PORT(%d),WEIGHT(%d)",
				svr.mGid, svr.mXid, svr.mHost, svr.mPort, svr.mWeight);

        return mStatus = wStatus::IOError("SvrQos::DeleteNode failed, cannot find the SvrNet_t", "");
    }

	mMutex.Lock();
    LOG_DEBUG(kSvrLog, "SvrQos::DeleteNode delete SvrNet_t success, GID(%d),XID(%d),HOST(%s),PORT(%d),WEIGHT(%d)",
			svr.mGid, svr.mXid, svr.mHost, svr.mPort, svr.mWeight);

    struct SvrStat_t* stat = mapReqIt->second;
    mMapReqSvr.erase(mapReqIt);

    if (DeleteRouteNode(svr) == -1) {
    	mStatus = wStatus::IOError("SvrQos::DeleteNode DeleteRouteNode() failed, the SvrNet_t delete failed", "");
    }
    SAFE_DELETE(stat);
	mMutex.Unlock();
    return mStatus;
}

const wStatus& SvrQos::GetNodeAll(struct SvrNet_t buf[], int32_t* num, int32_t start, int32_t size) {
	LOG_DEBUG(kSvrLog, "SvrQos::GetNodeAll get all SvrNet_t start, [%d, %d]", start, size);

	mMutex.Lock();
	*num = 0;
	MapSvrIt_t mapReqIt = mMapReqSvr.begin();
	for (std::advance(mapReqIt, start); mapReqIt != mMapReqSvr.end() && *num < size; mapReqIt++) {
		buf[(*num)++] = mapReqIt->first;
	}
	mMutex.Unlock();
	return mStatus;
}

const wStatus& SvrQos::NtyNodeSvr(const struct SvrNet_t& svr) {
	LOG_DEBUG(kSvrLog, "SvrQos::NtyNodeSvr SvrNet_t start, GID(%d),XID(%d),HOST(%s),PORT(%d)", svr.mGid, svr.mXid,svr.mHost,svr.mPort);
	
	mMutex.Lock();
	MapSvrIt_t mapReqIt = mMapReqSvr.find(svr);
	if (mapReqIt == mMapReqSvr.end()) {
		mStatus = wStatus::IOError("SvrQos::NtyNodeSvr NtyRouteNode() failed, cannot find the SvrNet_t", "");
	}
	struct SvrStat_t* stat = mapReqIt->second;
	stat->mInfo.mReqAll++;
	stat->mInfo.mSReqAll++;
	mMutex.Unlock();
	return mStatus;
}

const wStatus& SvrQos::QueryNode(struct SvrNet_t& svr) {
	LOG_DEBUG(kSvrLog, "SvrQos::QueryNode query SvrNet_t start, GID(%d),XID(%d)", svr.mGid, svr.mXid);

	mMutex.Lock();
	if (GetRouteNode(svr) < 0) {
		mStatus = wStatus::IOError("SvrQos::QueryNode GetRouteNode() failed, cannot find the SvrNet_t", "");
	}
	mMutex.Unlock();

	LOG_DEBUG(kSvrLog, "SvrQos::QueryNode query SvrNet_t end, GID(%d),XID(%d),HOST(%s),PORT(%d),WEIGHT(%d)",
			svr.mGid, svr.mXid, svr.mHost, svr.mPort, svr.mWeight);

#ifdef _DEBUG_
	SvrKind_t kind(svr);
	// 正常路由
	MapKindIt_t reIt = mRouteTable.find(kind);
	if (reIt != mRouteTable.end()) {
		MultiMapNode_t* mapNode = reIt->second;
		for (MultiMapNodeIt_t it = mapNode->begin(); it != mapNode->end() && it->second.mStat; it++) {
			struct SvrNet_t svr = it->second.mNet;
			struct SvrStat_t stat = *(it->second.mStat);

			LOG_DEBUG(kSvrLog, "SvrQos::QueryNode RouteTable,GID(%d),XID(%d),HOST(%s),PORT(%d),WEIGHT(%d),"
					"LOADX(%f),PreAll(%d),ReqLimit(%d),ReqCount(%d),ReqAll(%d),ReqSuc(%d),ReqErrRet(%d),ReqErrTm(%d),ReqRej(%d),"
					"OkLoad(%f),DelayLoad(%f),AvgTm(%d),OkRate(%f),AvgErrRate(%f),ReqErrMin(%f),ReqExtendRate(%f)",
					svr.mGid,svr.mXid,svr.mHost,svr.mPort,svr.mWeight,
					stat.mInfo.mLoadX,stat.mInfo.mPreAll,stat.mReqCfg.mReqLimit,stat.mReqCfg.mReqCount,stat.mInfo.mReqAll,
					stat.mInfo.mReqSuc,stat.mInfo.mReqErrRet,stat.mInfo.mReqErrTm,stat.mInfo.mReqRej,
					stat.mInfo.mOkLoad,stat.mInfo.mDelayLoad,stat.mInfo.mAvgTm,stat.mInfo.mOkRate,
					stat.mInfo.mAvgErrRate,stat.mReqCfg.mReqErrMin, stat.mReqCfg.mReqExtendRate);
		}
	}

	// 宕机路由
	MapNodeIt_t errIt = mErrTable.find(kind);
	if (errIt != mErrTable.end()) {
		ListNode_t* listNode = errIt->second;
		for (ListNodeIt_t it = listNode->begin(); it != listNode->end() && it->mStat; it++) {
			struct SvrNet_t svr = it->mNet;
			struct SvrStat_t stat = *(it->mStat);

			LOG_DEBUG(kSvrLog, "SvrQos::QueryNode errTable,GID(%d),XID(%d),HOST(%s),PORT(%d),WEIGHT(%d),"
					"LOADX(%f),PreAll(%d),ReqLimit(%d),ReqCount(%d),ReqAll(%d),ReqSuc(%d),ReqErrRet(%d),ReqErrTm(%d),ReqRej(%d),"
					"OkLoad(%f),DelayLoad(%f),AvgTm(%d),OkRate(%f),AvgErrRate(%f),ReqErrMin(%f),ReqExtendRate(%f)",
					svr.mGid,svr.mXid,svr.mHost,svr.mPort,svr.mWeight,
					stat.mInfo.mLoadX,stat.mInfo.mPreAll,stat.mReqCfg.mReqLimit,stat.mReqCfg.mReqCount,stat.mInfo.mReqAll,
					stat.mInfo.mReqSuc,stat.mInfo.mReqErrRet,stat.mInfo.mReqErrTm,stat.mInfo.mReqRej,
					stat.mInfo.mOkLoad,stat.mInfo.mDelayLoad,stat.mInfo.mAvgTm,stat.mInfo.mOkRate,
					stat.mInfo.mAvgErrRate,stat.mReqCfg.mReqErrMin, stat.mReqCfg.mReqExtendRate);
		}
	}
#endif

	return mStatus;
}

const wStatus& SvrQos::CallerNode(const struct SvrCaller_t& caller) {
	LOG_DEBUG(kSvrLog, "SvrQos::CallerNode report SvrCaller_t start, GID(%d),XID(%d),HOST(%s),PORT(%d),ReqRet(%d),ReqCount(%d),ReqUsetimeUsec(%lld)",
			caller.mCalledGid, caller.mCalledXid, caller.mHost, caller.mPort, caller.mReqRet, caller.mReqCount, caller.mReqUsetimeUsec);

	mMutex.Lock();
	if (ReportNode(caller) == -1) {
		mStatus = wStatus::IOError("SvrQos::CallerNode ReportNode() failed, cannot report the SvrNet_t", "");
	}
	mMutex.Unlock();

    return mStatus;
}

int SvrQos::ReportNode(const struct SvrCaller_t& caller) {
	if (caller.mCalledGid <= 0 || caller.mCalledXid <= 0 || caller.mPort <= 0 || caller.mHost[0] == 0) {
		LOG_ERROR(kSvrLog, "SvrQos::ReportNode report failed(caller data illegal), GID(%d),XID(%d),HOST(%s),PORT(%d)",
				caller.mCalledGid, caller.mCalledXid, caller.mHost, caller.mPort);

		return -1;
	}

	struct SvrNet_t svr;
	svr.mGid = caller.mCalledGid;
	svr.mXid = caller.mCalledXid;
	svr.mPort = caller.mPort;
	memcpy(svr.mHost, caller.mHost, kMaxHost);

	int64_t usec = caller.mReqUsetimeUsec;
	if (caller.mReqUsetimeUsec <= 0) {
		usec = 1;
	}

	MapSvrIt_t mapReqIt = mMapReqSvr.find(svr);
	MapSvrIt_t mapEndIt = mMapReqSvr.end();
    if (mapReqIt == mapEndIt) {
		LOG_ERROR(kSvrLog, "SvrQos::ReportNode report failed(cannot find caller), GID(%d),XID(%d),HOST(%s),PORT(%d)",
				caller.mCalledGid, caller.mCalledXid, caller.mHost, caller.mPort);

		return -1;
    }

    struct SvrStat_t* pSvrStat = mapReqIt->second;
    if (caller.mReqRet >= 0) {
    	// 成功
    	pSvrStat->mInfo.mReqSuc += caller.mReqCount;
    	pSvrStat->mInfo.mSReqSuc += caller.mReqCount;
    	pSvrStat->mInfo.mTotalUsec += usec;
    	pSvrStat->mInfo.mContErrCount = 0;
    } else {
    	// 失败
    	pSvrStat->mInfo.mReqErrRet += caller.mReqCount;
    	pSvrStat->mInfo.mSReqErrRet += caller.mReqCount;
    	pSvrStat->mInfo.mContErrCount += caller.mReqCount;
    }

    // 重建route
    struct SvrKind_t kind(svr);
    RebuildRoute(kind);
    
    return 0;
}

void SvrQos::LoadStatCfg(const struct SvrNet_t& svr, struct SvrStat_t* stat) {
	stat->mInfo.InitInfo(svr);
	stat->mReqCfg = mReqCfg;
}

int SvrQos::GetRouteNode(struct SvrNet_t& svr) {
	if (svr.mGid <= 0 || svr.mXid <= 0) {
		LOG_ERROR(kSvrLog, "SvrQos::GetRouteNode get failed(req SvrNet_t invalid),GID(%d),XID(%d)", svr.mGid, svr.mXid);
		return SVR_RTN_SYSERR;
	}
	struct SvrKind_t kind(svr);

	// 重建该类路由，并清理相关统计
	RebuildRoute(kind);

	MapKindIt_t rtIt = mRouteTable.find(kind);

	// 如果找不到相关(gid, xid)对应的路由，返回SVR_RTN_SYSERR
	if (rtIt == mRouteTable.end()) {
		LOG_ERROR(kSvrLog, "SvrQos::GetRouteNode get failed(the SvrNet_t not exists from mRouteTable), GID(%d),XID(%d)", svr.mGid, svr.mXid);
		// @TODO 反向注册路由，自动获取路由
		return SVR_RTN_SYSERR;
    }

	struct SvrKind_t& stKind = const_cast<struct SvrKind_t&>(rtIt->first);
    stKind.mAccess64tm = soft::TimeUsec();

	MultiMapNode_t* table = rtIt->second;
	if (!table || table->empty()) {
        SAFE_DELETE(table);
        mRouteTable.erase(rtIt);

		LOG_ERROR(kSvrLog, "SvrQos::GetRouteNode get failed(the SvrNet_t not exists(empty table)), GID(%d),XID(%d),HOST(%s),PORT(%d),WEIGHT(%d)",
				svr.mGid, svr.mXid, svr.mHost, svr.mPort, svr.mWeight);
		return SVR_RTN_OVERLOAD;
	}

	MultiMapNodeIt_t it = table->begin();

	// 已分配到第几个路由
	int32_t index = stKind.mPindex;
	if (index >= static_cast<int32_t>(table->size())) {
		stKind.mPindex = index = 0;
	}

	double firstReq = it->second.mKey * it->second.mStat->mInfo.mPreAll;
	std::advance(it, index);

	// 分配节点
	if (!index) {	// 非首节点
		double curAdjReq = it->second.mKey * it->second.mStat->mInfo.mPreAll;

		// 此处采用round_robin的经典算法，获得目标路由
		// 如果不是第一个路由,  获得比第一个路由的负载更低的路由
		while (curAdjReq >= firstReq) {
			do {
				++index;
				it = table->begin();
				if (index >= static_cast<int32_t>(table->size())) {	// 轮训到最后一个路由
					stKind.mPindex = index = 0;
					break;
				}
				std::advance(it, index);
			} while (it->second.mStat->mInfo.mOffSide == 1);

			if (index == 0) break;
			curAdjReq = it->second.mKey * it->second.mStat->mInfo.mPreAll;
			stKind.mPindex = index;
		}
	}

	// 未找到合适节点或第一个节点即为合适节点
	bool firstSvr = false;
	if (index == 0) {
		firstSvr = true;
	}

	// 已经获得了预选路由
	memcpy(svr.mHost, it->second.mNet.mHost, sizeof(svr.mHost));
	svr.mPort = it->second.mNet.mPort;
	svr.mWeight = it->second.mNet.mWeight;

    // 检测分配路由，如果有路由分配产生，则更新相关统计计数
	int ret;
	while ((ret = RouteCheck(it->second.mStat, svr, firstReq, firstSvr)) != SVR_RTN_ACCEPT) {
		index++;
		firstSvr = false;
		if (index >= static_cast<int32_t>(table->size())) {
			index = 0;
			firstSvr = true;
		}

		if (index == stKind.mPindex) {	// 检测到最后一个节点（下一个就是检测开始的首节点）
			it = table->begin();
			std::advance(it, index);	// 回到检测开始时的第一个节点

            if (ret == SVR_RTN_OFFSIDE) {	// 最后一个节点为远程节点，强制分配检测开始的首节点
            	memcpy(svr.mHost, it->second.mNet.mHost, sizeof(svr.mHost));
            	svr.mPort = it->second.mNet.mPort;
            	svr.mWeight = it->second.mNet.mWeight;

        		LOG_ERROR(kSvrLog, "SvrQos::GetRouteNode get success off_side_node SvrNet_t, GID(%d),XID(%d),HOST(%s),PORT(%d),WEIGHT(%d),INDEX(%d)",
        				svr.mGid, svr.mXid, svr.mHost, svr.mPort, svr.mWeight, index);

            	break;
            } else {	// 最后节点还是过载
                it->second.mStat->mInfo.mReqRej++;
                it->second.mStat->mInfo.mSReqRej++;

        		LOG_ERROR(kSvrLog, "SvrQos::GetRouteNode get failed(all SvrNet_t overload), GID(%d),XID(%d),HOST(%s),PORT(%d),WEIGHT(%d),ReqRej(%d)",
        				svr.mGid, svr.mXid, svr.mHost, svr.mPort, svr.mWeight, it->second.mStat->mInfo.mReqRej);

                return SVR_RTN_OVERLOAD;
            }
		}

		it = table->begin();
		std::advance(it, index);
		memcpy(svr.mHost, it->second.mNet.mHost, sizeof(svr.mHost));
		svr.mPort = it->second.mNet.mPort;
		svr.mWeight = it->second.mNet.mWeight;
	}

	// 节点分配过期时间
	//time_t tm = soft::TimeUnix();
	//svr.mExpired = tm + stKind.mRebuildTm - (tm - stKind.mPtm);
	
	
	/** 滚动路由，激活分配节点WRR算法（由下一次分配使用） */

	// 如果是第一个路由, 尝试探测分配第二个路由（始终会滚动到该路由）
	if (it == table->begin()) {
		it++;
		if (it == table->end()) {	// 只有一个路由~
			return SVR_RTN_ACCEPT;
		}

		// 尝试获取负载更低的路由
		firstReq = table->begin()->second.mKey * table->begin()->second.mStat->mInfo.mPreAll;
		if (it->second.mStat->mInfo.mPreAll * it->second.mKey < firstReq) {
			if (index + 1 < static_cast<int32_t>(table->size())) {
				index++;
			} else {
				index = 0;
			}
			stKind.mPindex = index;
		}
		return SVR_RTN_ACCEPT;
	}

	// 如果不是第一个路由，且该负载比第一个路由大，则下次分配滚动到下一个路由
	firstReq = table->begin()->second.mKey * table->begin()->second.mStat->mInfo.mPreAll;
	if (it->second.mStat->mInfo.mPreAll * it->second.mKey >= firstReq) {
		if ((index + 1) < static_cast<int32_t>(table->size())) {
			stKind.mPindex = index + 1;
		} else {
			stKind.mPindex = 0;
		}
	}

	LOG_DEBUG(kSvrLog, "SvrQos::GetRouteNode get success, GID(%d),XID(%d),HOST(%s),PORT(%d),WEIGHT(%d),index(%d)",
			svr.mGid, svr.mXid, svr.mHost, svr.mPort, svr.mWeight, stKind.mPindex);
	return SVR_RTN_ACCEPT;
}

void SvrQos::RebuildRoute(struct SvrKind_t& kind, bool force) {
	MapKindIt_t rtIt = mRouteTable.find(kind);
	if (rtIt == mRouteTable.end()) {
		mAllReqMin = true;	// 全部过载

		// 重建宕机路由，恢复拉起SVR节点
		MultiMapNode_t* newTable;
		SAFE_NEW(MultiMapNode_t, newTable);
        RebuildErrRoute(kind, newTable, 1, 1, 1);

        if (!newTable->empty()) {	// 拉起节点
        	kind.mInnerChange++;
        	kind.mPindex = 0;
            mRouteTable.insert(std::make_pair(kind, newTable));
        } else {
            SAFE_DELETE(newTable);
        }
        return;
	}

	MultiMapNode_t* table = rtIt->second;	// 节点列表
	if (!table || table->empty()) {
        SAFE_DELETE(table);
        mRouteTable.erase(rtIt);
        return;
	}
	struct SvrKind_t& stKind = const_cast<struct SvrKind_t&>(rtIt->first);

	time_t tm = soft::TimeUnix();
	// 防止时间跳变 （服务器时钟故障 || 请求某sid路由间隔超过两周期不重建路由）
	if ((tm - stKind.mPtm) > 2*stKind.mRebuildTm || (stKind.mPtm - tm) > 2*stKind.mRebuildTm) {
		stKind.mPtm = tm;
	}
	// 如果非强制更新，并且还没到路由rebuild时间直接返回，默认一分钟rebuild一次
    if (!force && (tm - stKind.mPtm < stKind.mRebuildTm)) {
    	return;
    }

    // 更新rebuild绝对时间
    kind.mPtm = stKind.mPtm = tm;

    bool errRateBig = true;  // 所有被调都过载标志
    int32_t routeTotalReq = 0; // 总请求数实际为分配路由给客户端数数量

    uint64_t lowDelay = kDelayMax;	// 路由最低延时时间
    float heightSucRate = 0.0f;		// 路由最高成功率
    float totalErrRate = 0.0f;		// 路由失败率总和
    float highWeight = 0.0f;		// 路由最高权重值
	float cfgErrRate = 0.0f;		// 访问量控制错误率最小值

    // update统计数量
    for (MultiMapNodeIt_t it = table->begin(); it != table->end(); it++) {
    	struct SvrInfo_t &info = it->second.mStat->mInfo;

    	// 备份本周期数据
    	int32_t reqAll = info.mReqAll;
    	int32_t reqRej = info.mReqRej;
    	int32_t reqSuc = info.mReqSuc;
    	int32_t reqErrRet = info.mReqErrRet;
    	int32_t reqErrTm = info.mReqErrTm;
    	routeTotalReq += reqAll - reqRej;
    	if (reqAll < reqSuc + reqErrRet + reqErrTm) {	// 统计不一致
    		reqAll = reqSuc + reqErrRet + reqErrTm;
    	}

    	// 使用已有统计数据计算负载均衡各项数据：门限控制、计算成功率、平均延时值
    	RouteNodeRebuild(it->second.mNet, it->second.mStat);

    	cfgErrRate = it->second.mStat->mReqCfg.mReqErrMin;
    	float nodeErrRate = 1 - info.mOkRate;	// 路由实际错误率
    	
    	// 有效错误率 = 实际错误率 - 平均错误率
    	if (it->second.mStat->mReqCfg.mReqErrMin > nodeErrRate - info.mAvgErrRate) {	// 有效错误率未超过阈值
    		errRateBig = false;	// 说明节点可用（所有节点错误率都很高而已。节点本身有问题？）
    	}

    	if (nodeErrRate < it->second.mStat->mReqCfg.mReqErrMin)	{	// 错误率未超过阈值。节点正常，清除连续过载标志
    		stKind.mPsubCycCount = 0;
    		stKind.mPtotalErrRate= 0;
    	} else {	// 节点过载
            char alartInfo[256];
            ::snprintf(alartInfo, sizeof(alartInfo) -1 , "SvrQos::RebuildRoute one SvrNet_t ready to overload,GID(%d),XID(%d),HOST(%s),PORT(%d),"
            		"ReqLimit(%d) ReqAll(%d) ReqSuc(%d) ReqRej(%d) reqErrRet(%d) reqErrTm(%d) server SvrNet_t errRate(%f)(%f) > configure ERR_RATE(%f)",
					stKind.mGid, stKind.mXid, it->second.mNet.mHost, it->second.mNet.mPort,
					it->second.mStat->mReqCfg.mReqLimit, reqAll, reqSuc, reqRej, reqErrRet, reqErrTm, nodeErrRate, info.mAvgErrRate, it->second.mStat->mReqCfg.mReqErrMin);

    		LOG_ERROR(kSvrLog, alartInfo);
    		// SendMobileMsg();
    	}

    	misc::GetTimeofday(&info.mBuildTm);	// 重建绝对时间

    	// 重置权重值
    	if (it->second.mNet.mWeight == 0) {
    		it->second.mNet.mWeight = kInitWeight;
    	}

    	totalErrRate += nodeErrRate; // 路由失败率总和

    	//lowDelay = std::min(lowDelay, info.mAvgTm);	// 路由最低延时时间
    	//heightSucRate = std::max(heightSucRate, info.mOkRate);	// 路由最高成功率
    	//highWeight = std::max(highWeight, it->second.mNet.mWeight);	// 路由最高权重值
    	heightSucRate = heightSucRate >= info.mOkRate? heightSucRate: info.mOkRate;	// 路由最高成功率
    	lowDelay = lowDelay >= info.mAvgTm? info.mAvgTm: lowDelay; // 路由最低延时时间
    	highWeight = highWeight >= it->second.mNet.mWeight? highWeight: it->second.mNet.mWeight; // 路由最高权重值
    }
    if (lowDelay <= 0) {
    	lowDelay = 1;
    }

    if (errRateBig) {	// 全部节点都过载（非压力故障）
    	// 计算所有节点平均连续过载错误率、连续过载次数
    	float avgErrRate = totalErrRate / table->size();
    	stKind.mPtotalErrRate += avgErrRate;
    	stKind.mPsubCycCount++;

    	// 该sid下所有节点过载平均错误率
    	mAvgErrRate = stKind.mPtotalErrRate / stKind.mPsubCycCount;

    	char alartInfo[256];
        ::snprintf(alartInfo, sizeof(alartInfo) -1 , "SvrQos::RebuildRoute all SvrNet_t ready to overload,GID(%d),XID(%d),"
        		"avgErrRate(%f) server all SvrNet_t errRate > configure ERR_RATE(%f)",
				stKind.mGid, stKind.mXid, avgErrRate, cfgErrRate);

        LOG_ERROR(kSvrLog, alartInfo);
	    // SendMobileMsg();
    }

    // 更新节点宕机后请求数
    MapNodeIt_t reIt = mErrTable.find(kind);
	if (reIt != mErrTable.end()) {
		for (ListNodeIt_t eit = reIt->second->begin(); eit != reIt->second->end(); eit++) {
			// 累积宕机的服务自宕机开始，该sid下的所有节点请求数
			// 此请求数用来作为判断是否进行宕机恢复探测的依据
			eit->mReqAllAfterDown += routeTotalReq;
		}
	}

	mAllReqMin = true;

	MultiMapNode_t* newTable;
	SAFE_NEW(MultiMapNode_t, newTable);

	float bestLowPri = 1.0f;	// 最大负载
    float bestLowSucRate = 1.0f;// 最小成功率
    uint32_t bestBigDelay = 1;	// 最大延时值

	// 计算负载
	for (MultiMapNodeIt_t it = table->begin(); it != table->end(); it++) {
		struct SvrStat_t* pStat = it->second.mStat;
		struct SvrInfo_t& info = pStat->mInfo;

        cfgErrRate = pStat->mReqCfg.mReqErrMin;

        // 所有节点能力相当情况下，理论上成功率负载、时延负载 都趋近于1
        info.mDelayLoad = static_cast<float>(info.mAvgTm) / static_cast<float>(lowDelay);	// 延时率负载
        if (info.mOkRate > 0) {
        	info.mOkLoad = heightSucRate / info.mOkRate;
        } else {
        	info.mOkLoad = kOkLoadMax;
        }
        info.mOkLoad = info.mOkLoad >= 1 ? info.mOkLoad : 1;	// 成功率负载
        float weightLoad = static_cast<float>(highWeight)/static_cast<float>(it->second.mNet.mWeight);	// 静态权重负载
        
        // 系统配置的负载 比例因子
        if (mRateWeight == mDelayWeight) {
        	info.mLoadX = info.mDelayLoad * info.mOkLoad;
        } else {
        	info.mLoadX = info.mDelayLoad*mDelayWeight + info.mOkLoad*mRateWeight;
        }

        // 判断是否就近接入 mOffSide=0 就近 mOffSide=1 非就近
        // sid需要就近接入，并且主被调同城，即就近
        if (/*stKind.mNearestAccessFlag &&*/ mIdc > 0 && mIdc != info.mCityId) {
        	info.mOffSide = 1;
        } else {
        	info.mOffSide = 0;
        }

        info.mLoadX *= weightLoad;
        info.NextCycReady();

        if (info.mOffSide == 0) {	// 只计算同城就近接入节点
        	//bestLowPri = std::max(bestLowPri, info.mLoadX);
        	bestLowPri = bestLowPri >= info.mLoadX? bestLowPri: info.mLoadX;
        }

        // 恢复路由 || 宕机路由
        struct SvrNode_t node(it->second.mNet, pStat);	// set IsDetecting = false
        int32_t reqLimit = pStat->mReqCfg.mReqLimit <= 0 ? (pStat->mReqCfg.mReqMin + 1) : pStat->mReqCfg.mReqLimit;

        if (reqLimit > pStat->mReqCfg.mReqMin) {	// 门限大于最低阈值，加入节点路由
        	if (info.mOffSide == 0) {	// 只计算同城就近接入节点
            	//bestLowSucRate = std::min(bestLowSucRate, node.mStat->mInfo.mOkRate);
            	//bestBigDelay = std::max(bestBigDelay, node.mStat->mInfo.mAvgTm);
            	bestLowSucRate = bestLowSucRate >= node.mStat->mInfo.mOkRate? node.mStat->mInfo.mOkRate: bestLowSucRate;
            	bestBigDelay = bestBigDelay >= node.mStat->mInfo.mAvgTm? bestBigDelay: node.mStat->mInfo.mAvgTm;
        	}
        	newTable->insert(std::make_pair(node.mKey, node));
        	mAllReqMin = false;
        } else {	// 门限小于最低阈值，加入宕机列表
            AddErrRoute(kind, node);
        }
	}

	// 所有节点都过载
	if (mAllReqMin) {
        if (stKind.mPsubCycCount <= 0) {
        	stKind.mPsubCycCount = 1;
        }
        // 该sid下所有节点过载平均错误率
        mAvgErrRate = stKind.mPtotalErrRate / stKind.mPsubCycCount;

        if (mAvgErrRate > 0.99999) {
        	mAvgErrRate = 0;
        }

        // 过载平均错误率
        if (mAvgErrRate > static_cast<float>(1 - cfgErrRate)) {
            mAvgErrRate = 1 - cfgErrRate - 0.1;
            if (mAvgErrRate < 0) {
            	mAvgErrRate = 0;
            }
        }

        char alartInfo[512];
        ::snprintf(alartInfo, sizeof(alartInfo) -1 , "SvrQos::RebuildRoute all SvrNet_t overload,GID(%d),XID(%d),"
        		"avgErrRate(%f) server all SvrNet_t minErr > configure ERR_RATE(%f)",
				stKind.mGid, stKind.mXid, mAvgErrRate, cfgErrRate);

        LOG_ERROR(kSvrLog, alartInfo);
	    // SendMobileMsg();
	}

	// 重建宕机路由
    RebuildErrRoute(kind, newTable, bestLowPri, bestLowSucRate, bestBigDelay);

    struct SvrKind_t newKind(stKind);
    newKind.mPindex = 0;
    newKind.mInnerChange++;

    // 重新添加
    mRouteTable.erase(rtIt);
    SAFE_DELETE(table);
    mRouteTable.insert(std::make_pair(newKind, newTable));
}

int SvrQos::AddErrRoute(struct SvrKind_t& kind, struct SvrNode_t& node) {
	LOG_DEBUG(kSvrLog, "SvrQos::AddErrRoute add SvtNet_t into ErrTable start, GID(%d),XID(%d),HOST(%s),PORT(%d),WEIGHT(%d),reqLimit(%d),avgTm(%d),okRate(%f),loadX(%f)",
			kind.mGid, kind.mXid, node.mNet.mHost, node.mNet.mPort, node.mNet.mWeight, node.mStat->mReqCfg.mReqLimit, node.mStat->mInfo.mAvgTm, node.mStat->mInfo.mOkRate, node.mStat->mInfo.mLoadX);

	ListNode_t* ListRoute = NULL;
	MapNodeIt_t reIt = mErrTable.find(kind);
    if (reIt == mErrTable.end()) {
        SAFE_NEW(ListNode_t, ListRoute);
        kind.mRebuildTm = node.mStat->mReqCfg.mRebuildTm;
        mErrTable.insert(std::make_pair(kind, ListRoute));
    } else {
    	ListRoute = reIt->second;
	}

	// record down node
    node.mStopTime = static_cast<int32_t>(time(NULL));
	node.mReqAllAfterDown = 0;
    ListRoute->push_back(node);
    return 0;
}

// 重建宕机路由列表
void SvrQos::RebuildErrRoute(struct SvrKind_t& kind, MultiMapNode_t* multiNode, float maxLoad, float lowOkRate, uint32_t bigDelay) {
	MapNodeIt_t reIt = mErrTable.find(kind);
    if (reIt == mErrTable.end()) {
        return;
    }

    ListNode_t* errRoute = reIt->second;
    if (!errRoute && errRoute->empty()) {
        SAFE_DELETE(errRoute);
		mErrTable.erase(reIt);

    	LOG_DEBUG(kSvrLog, "SvrQos::RebuildErrRoute cannot find errRoute table, GID(%d),XID(%d),", kind.mGid, kind.mXid);
		return;
	}

    // 全部过载，宕机节点恢复，负载设为最小
    if (mAllReqMin) {
    	maxLoad = 1;
	}

	ListNodeIt_t it = errRoute->begin();
    
    // 宕机检测、故障恢复
    int32_t ret, detectStat;
    int32_t tm = soft::TimeUnix();
	bool del_flag = false;
    DetectThread::VecNode_t detectNodeadd, detectNodedel;

	while (it != errRoute->end()) {
		del_flag = false;
		detectStat = -1;	// -1:网络探测失败 0:网络探测成功

		// 故障探测（mProbeBegin > 0 才打开网络层探测）
		//	1) 机器宕机后该sid已经收到超过一定量的请求（默认10000）
		//	2) 机器宕机已经超过一定时间，需要进行一次探测（默认3s）
		struct DetectResult_t res;
		if (mDownCfg.mProbeBegin > 0 && ((tm > it->mStopTime + mDownCfg.mProbeBegin) || (it->mReqAllAfterDown >= mDownCfg.mReqCountTrigerProbe))) {

			// 先进行网络层探测ping connect udp_icmp，网络探测成功后再用业务请求进行探测
			struct DetectNode_t detectNode(it->mNet.mHost, it->mNet.mPort, tm, tm + mDownCfg.mProbeNodeExpireTime);

			ret = mDetectThread->GetDetectResult(detectNode, res);
			if (ret < 0) {	// 无探测节点
				detectNodeadd.push_back(detectNode);
			} else if (ret == 0) {	// 已探测
				if (res.mRc < 0) {	// 探测失败
					detectStat = -1;
				} else if (res.mRc == 0 && (res.mDetectType == DETECT_TCP || res.mDetectType == DETECT_UDP)) {	// 探测协议

					// 删除探测节点
					detectNodedel.push_back(detectNode);
					del_flag = true;
					detectStat = 0;
				} else {	// 非法类型
					detectStat = -1;
				}
			}
		}

		// 没有全部过载，且没达到恢复条件之前，如果网络探测成功,提前恢复
		if (!mAllReqMin && (tm - it->mStopTime < mDownCfg.mDownTimeTrigerProbe) && (it->mReqAllAfterDown < mDownCfg.mReqCountTrigerProbe)) {
			if (0 != detectStat) {
				it++;
				continue;
			}
		}

		// 网络探测失败，推迟恢复
		if (mDownCfg.mProbeBegin > 0 && detectStat == -1) {
	        it++;
	        continue;
		}

		it->mIsDetecting = true;	// 刚刚从故障机列表中放到正常机器列表，处在探测状态
		it->mKey = maxLoad;
		it->mStat->mReqCfg.mReqLimit = 0; // = mDownCfg.mProbeTimes;

		if (mAllReqMin) {
			// 全部过载
            it->mStat->mInfo.mAvgErrRate = mAvgErrRate;
            it->mStat->mInfo.mLoadX = 1;
            it->mStat->mInfo.mOkLoad = 1;
            it->mStat->mInfo.mDelayLoad = 1;
        } else {
            it->mStat->mInfo.mLoadX = maxLoad;
            it->mStat->mInfo.mOkRate = lowOkRate;
            it->mStat->mInfo.mAvgTm = bigDelay;
        }

		// 统计置0
		it->mStat->mInfo.mReqAll = 0;
		it->mStat->mInfo.mSReqAll = 0;
		it->mStat->mInfo.mReqRej = 0;
		it->mStat->mInfo.mSReqRej = 0;
		it->mStat->mInfo.mReqSuc = 0;
		it->mStat->mInfo.mSReqSuc = 0;
		it->mStat->mInfo.mReqErrRet = 0;
		it->mStat->mInfo.mSReqErrRet = 0;
		it->mStat->mInfo.mReqErrTm = 0;
		it->mStat->mInfo.mSReqErrTm = 0;

		if (!del_flag) {
			detectNodedel.push_back(DetectNode_t(it->mNet.mHost, it->mNet.mPort, tm, tm + mDownCfg.mProbeNodeExpireTime));
		}
		LOG_DEBUG(kSvrLog, "SvrQos::RebuildErrRoute recover one route, GID(%d),XID(%d),HOST(%s),PORT(%d)", kind.mGid, kind.mXid, it->mNet.mHost, it->mNet.mPort);

		multiNode->insert(std::make_pair(maxLoad, *it));
		errRoute->erase(it++);
	}

    if (errRoute->empty()) {
        SAFE_DELETE(errRoute);
		mErrTable.erase(reIt);
	}

    if (!detectNodeadd.empty()) {
    	mDetectThread->AddDetect(detectNodeadd);
    }
    if (!detectNodedel.empty()) {
    	mDetectThread->DelDetect(detectNodedel);
    }
	return;
}

int32_t SvrQos::GetAddCount(const struct SvrStat_t* stat, int32_t reqCount) {
    if (stat->mInfo.mLastErr) {	// 上周期存在过度扩张
        return std::max(((stat->mInfo.mLastAlarmReq - reqCount) * stat->mReqCfg.mReqExtendRate), static_cast<float>(stat->mReqCfg.mReqMin));
    } else {	// 上周期不存在过度扩张
        return std::max((stat->mReqCfg.mReqLimit * stat->mReqCfg.mReqExtendRate / stat->mInfo.mDelayLoad), static_cast<float>(stat->mReqCfg.mReqMin));
    }
}

void SvrQos::RouteNodeRebuild(const struct SvrNet_t &svr, struct SvrStat_t* stat) {
	return ReqRebuild(svr, stat);
}

void SvrQos::ReqRebuild(const struct SvrNet_t &svr, struct SvrStat_t* pSvrStat) {
	int32_t sucCount = pSvrStat->mInfo.mReqSuc;
	int32_t errCount = pSvrStat->mInfo.mReqErrRet + pSvrStat->mInfo.mReqErrTm;

    if (pSvrStat->mInfo.mReqAll < errCount + sucCount) {	// 防止核算错误
        pSvrStat->mInfo.mReqAll = errCount + sucCount;
    }
    int32_t reqAll = pSvrStat->mInfo.mReqAll;

    // 保存到上一周期数据
    pSvrStat->mInfo.mLastReqAll = reqAll;
    pSvrStat->mInfo.mLastReqRej = pSvrStat->mInfo.mReqRej;
    pSvrStat->mInfo.mLastReqErrRet = pSvrStat->mInfo.mReqErrRet;
    pSvrStat->mInfo.mLastReqErrTm = pSvrStat->mInfo.mReqErrTm;
    pSvrStat->mInfo.mLastReqSuc = pSvrStat->mInfo.mReqSuc;
    if (pSvrStat->mInfo.mLastReqSuc < 0) {
		pSvrStat->mInfo.mLastReqSuc = 0;
	}

	// 本周期无请求
    if (reqAll <= 0 && errCount + sucCount <= 0) {
    	pSvrStat->mInfo.mOkRate = 1.0f;	// 重置成功率为1
    	pSvrStat->Reset();	// 重置统计信息 mAvgTm not change
        return;
    }

    // 失败率
    float errRate = reqAll > 0 ? static_cast<float>(errCount)/static_cast<float>(reqAll) : 0;
    if (isnan(errRate)) {
    	errRate = 0;
    }
    errRate = errRate < 1 ? errRate : 1;
    sucCount = sucCount > 0 ? sucCount : 0;
    pSvrStat->mInfo.mOkRate = 1 - errRate;

	// 核算节点平均延时值
    if (pSvrStat->mInfo.mReqSuc > 0) {	// 有成功的请求
        pSvrStat->mInfo.mAvgTm = pSvrStat->mInfo.mTotalUsec/pSvrStat->mInfo.mReqSuc;
    } else {
        if (reqAll > pSvrStat->mReqCfg.mReqMin) {	// 请求数 超过 请求最小阈值，延时设置最大值
            pSvrStat->mInfo.mAvgTm = kDelayMax;	// 100s 最大值延时值
        } else {
        	// 参照上周期平均延时核算本周期平均延时
            if (pSvrStat->mInfo.mAvgTm && pSvrStat->mInfo.mAvgTm < kDelayMax) {	// 延时增加一倍
                pSvrStat->mInfo.mAvgTm = 2 * pSvrStat->mInfo.mAvgTm;
            } else {
                pSvrStat->mInfo.mAvgTm = kDelayMax / 100;	// 1s 默认延时值
            }
		}
	}

	// 初始化门限值
	if (pSvrStat->mReqCfg.mReqLimit <= 0) {
		if (reqAll > pSvrStat->mReqCfg.mReqMin) {
			pSvrStat->mReqCfg.mReqLimit = reqAll;
		} else {	// 最低合法门限
			pSvrStat->mReqCfg.mReqLimit = pSvrStat->mReqCfg.mReqMin + 1;
		}
	}
	pSvrStat->mReqCfg.mReqCount = pSvrStat->mInfo.mReqAll;

    if (errRate <= pSvrStat->mReqCfg.mReqErrMin) {	// 节点错误率未超门限，重置节点平均失败率为0
    	pSvrStat->mInfo.mAvgErrRate = 0;
    }

	// 路由的过载判断逻辑修改为：
	// 路由的当前周期错误率超过配置的错误率阀值时才判断为过载
    
    // 门限控制mReqLimit
    // 有效错误率 = 实际错误率 - 平均错误率
    if (pSvrStat->mReqCfg.mReqErrMin >=  errRate - pSvrStat->mInfo.mAvgErrRate) {	// 有效错误率小于一定的比例，认同为成功的情况
    	// 如果成功数大于上次过度扩张时的门限，则过度扩张标志无效
        if (pSvrStat->mInfo.mLastErr && sucCount > pSvrStat->mInfo.mLastAlarmReq) {
            pSvrStat->mInfo.mLastErr = false;
        }

        // 门限期望值： reqAll * (1 + mReqExtendRate)
        int32_t dest = reqAll + static_cast<int>(reqAll * mReqCfg.mReqExtendRate);

        if (pSvrStat->mReqCfg.mReqLimit < dest) {
            pSvrStat->mReqCfg.mReqLimit += GetAddCount(pSvrStat, pSvrStat->mInfo.mReqAll);
            if (pSvrStat->mReqCfg.mReqLimit > dest) {	// 门限大于预定义的req_max，设置门限为req_max
                pSvrStat->mReqCfg.mReqLimit = dest;
            }
        }
    } else {	// 错误率大于一定的比例，认同失败的情况，需要按失败率收缩门限
		if (pSvrStat->mInfo.mOkRate > 0) {
			errRate -= pSvrStat->mInfo.mAvgErrRate;	// 有效错误率
		}

		// 是否直接收缩至成功数？：
		// 1. 因为宕机时直接缩到成功数，可能会使得下个周期的门限比其他机器的大
		// 2. 所以门限值按照 req_limit 和 req_count 中较小的一个进行收缩
		if (reqAll > pSvrStat->mReqCfg.mReqLimit) {
			pSvrStat->mReqCfg.mReqLimit = pSvrStat->mReqCfg.mReqLimit - static_cast<int32_t>(pSvrStat->mReqCfg.mReqLimit * errRate);
		} else {
			pSvrStat->mReqCfg.mReqLimit = reqAll - static_cast<int32_t>(reqAll * errRate);
		}

		// 记录过度扩展阈值
		// 逻辑上可认为，因为上个周期门限的扩张，从而使本周期的访问量增大，却又导致了本周期错误率超过阈值
		// 也就说明本周期扩展后的请求数至少为该路由在本周期的错误负责。即最大警报值
		pSvrStat->mInfo.mLastErr = true;
		pSvrStat->mInfo.mLastAlarmReq = reqAll;
		pSvrStat->mInfo.mLastAlarmSucReq = sucCount;
		if (reqAll <= pSvrStat->mReqCfg.mReqMin) {	// 请求数需大于最小请求数统计才有效
			pSvrStat->mInfo.mLastErr = false;
		}
	}

    // 宕机 "嫌疑" 判定：
    // mReqLimit 大于最小值+1，并且：
	// 	1) 连续失败次数累加达到设定值，则该机器有宕机嫌疑
	// 	2) 错误率大于一定的比例，则该机器有宕机嫌疑
	if (pSvrStat->mReqCfg.mReqLimit > pSvrStat->mReqCfg.mReqMin + 1 &&
			((pSvrStat->mInfo.mContErrCount >= mDownCfg.mPossibleDownErrReq) || (errRate > mDownCfg.mPossbileDownErrRate))) {

		pSvrStat->mReqCfg.mReqLimit = pSvrStat->mReqCfg.mReqMin + 1;	// 宕机门限设置最小值

		LOG_DEBUG(kSvrLog, "SvrQos::ReqRebuild continuous GID(%d),XID(%d),HOST(%s),PORT(%d),ReqLimit(%d)", svr.mGid, svr.mXid, svr.mHost, svr.mPort, pSvrStat->mReqCfg.mReqLimit);
	}

    // 硬限制：不可超出[REQ_MIN, REQ_MAX]的范围
    if (pSvrStat->mReqCfg.mReqLimit > pSvrStat->mReqCfg.mReqMax) {
        pSvrStat->mReqCfg.mReqLimit = pSvrStat->mReqCfg.mReqMax;
    }
    if (pSvrStat->mReqCfg.mReqLimit < pSvrStat->mReqCfg.mReqMin) {
    	// 设置为最小值（注意：不是最小值+1），会被加入宕机列表
        pSvrStat->mReqCfg.mReqLimit = pSvrStat->mReqCfg.mReqMin;
    }

    // 重置统计信息 mAvgTm not change
    pSvrStat->Reset();
    return;
}

int SvrQos::RouteCheck(struct SvrStat_t* stat, struct SvrNet_t& svr, double firstLoad, bool firstSvr, pid_t pid) {
	// 只计算就近接入节点
	if (stat->mInfo.mOffSide == 1) {
		return SVR_RTN_OFFSIDE;
	}

	// 更新访问量
	stat->mInfo.mReqAll++;
    stat->mInfo.mSReqAll++;
	
	// 路由预取数
    //svr.mPre = 1;

    // 记录获得路由的执行体ID，并递增该执行体的路由申请计数
	struct PidInfo_t* pidInfo = NULL;
	if (pid) {
		pidInfo = &(stat->mInfo.mClientInfo[pid]);
		pidInfo->mReq++; // fixed: check
		pidInfo->mTid = pid;
	}

    // 门限<=0（可能是首个周期）
    if (stat->mReqCfg.mReqLimit <= 0) {
    	stat->mInfo.mPreAll++;
	    stat->mInfo.mSPreAll++;
        return SVR_RTN_ACCEPT;
    }

    // 本周期统计信息
    int32_t reqCount = stat->mInfo.mReqAll;
    int32_t errCount = stat->mInfo.mReqErrRet + stat->mInfo.mReqErrTm;
    float errRate = static_cast<float>(errCount)/static_cast<float>(reqCount);

    // 当前周期访问量超过阀值
    if (reqCount >= stat->mReqCfg.mReqLimit) {
    	// 且当前周期的错误率超过配置的错误率阀值时才判断为过载
		if (errRate > stat->mReqCfg.mReqErrMin) {
			if (pid && pidInfo) {	// 记录进程相关统计值
				pidInfo->mRej++; // fixed: check
				pidInfo->mTid = pid;
			}
			stat->mInfo.mReqAll--;
		    stat->mInfo.mSReqAll--;

            stat->mInfo.mPreAll++;
            stat->mInfo.mSPreAll++;

		    // 过载
	        LOG_ERROR(kSvrLog, "SvrQos::RouteCheck check failed(the SvrNet_t overload),GID(%d),XID(%d),HOST(%s),PORT(%d)", svr.mGid, svr.mXid, svr.mHost, svr.mPort);
            return SVR_RTN_OVERLOAD;
        }

		stat->mInfo.mPreAll++;
		stat->mInfo.mSPreAll++;
        return SVR_RTN_ACCEPT;
    }

    /** @TODO
	// 不是第一个节点
    if (!firstSvr) {
    	double curAllReq = stat->mInfo.mPreAll * stat->mInfo.mLoadX;

    	// 当前负载高于第一个负载，分配之。预取数为1
        if (curAllReq >= firstLoad) {
        	stat->mInfo.mPreAll++;
			stat->mInfo.mSPreAll++;
	        return SVR_RTN_ACCEPT;
        }
	}
	*/

	/**
	int32_t pidLastsuc = 0;
	int32_t pidInc   =  0;
	int32_t pidErrcount = 0;
	int32_t pidReq = 0;
	float pidErate = 0.0;
	int32_t pidCycTm = 0;

	if (pidInfo) {
		pidLastsuc = pidInfo->GetLastSuc();
		pidInc     = pidInfo->GetIncremental();
		pidErrcount = pidInfo->mErr;
		pidReq     = pidInfo->mReq;
		pidCycTm  = pidInfo->mCycTm;
	} else {
		pidLastsuc = stat->mInfo.mClientInfo[pid].GetLastSuc();
		pidInc   =  stat->mInfo.mClientInfo[pid].GetIncremental();
		pidErrcount = stat->mInfo.mClientInfo[pid].mErr;
		pidReq = stat->mInfo.mClientInfo[pid].mReq;
		pidCycTm = stat->mInfo.mClientInfo[pid].mCycTm;
	}

	if (pidReq > 0) {
		pidErate = static_cast<float>(pidErrcount) / static_cast<float>(pidReq);
	}

	// 若某进程的错误率高于预设的最低错误率，则返回的路由预取数为1
    if (pidErate > stat->mReqCfg.mReqErrMin + stat->mInfo.mAvgErrRate) {
		stat->mInfo.mPreAll++;
		stat->mInfo.mSPreAll++;
        return SVR_RTN_ACCEPT;
    }
	*/

	// 若节点的有效错误率高于预设的最低错误率，返回的路由。预取数为1
	if (errRate > stat->mReqCfg.mReqErrMin + stat->mInfo.mAvgErrRate) {
		stat->mInfo.mPreAll++;
		stat->mInfo.mSPreAll++;
        return SVR_RTN_ACCEPT;
	}

	/**
	// 计算预取数
    // 初始值为预测的本周期的成功请求数
    // 算法：上个周期的成功请求数 加上 上个周期相对于上上个周期成功请求数的差值，在一个预取时间段（qos.xml中PRE_TIME的值，默认为4秒）内的部分
    int suc = mReqCfg.mPreTime * ((pidLastsuc + pidInc) / stat->mReqCfg.mRebuildTm);
    float errate = 1 - stat->mInfo.mOkRate - stat->mInfo.mAvgErrRate;	// 节点有效错误率

    // 预取数收缩
    if (errate > 0 && errate < 1) {
    	suc -= static_cast<int32_t>(suc * errate);
    	if (stat->mInfo.mLoadX > 1) {
    		suc = static_cast<int32_t>(suc / stat->mInfo.mLoadX);
    	}
    }

    // 构建时间间隔多余120s
    if (soft::TimeUnix() - pidCycTm > 120) {
    	suc = 1;
    }

    // 已分配数 + 预取数 > mReqLimit 则从扣除超过部分
    if (stat->mInfo.mPreAll + suc > stat->mReqCfg.mReqLimit) {
        int32_t limv = (stat->mInfo.mPreAll  + suc  - stat->mReqCfg.mReqLimit) / stat->mInfo.mClientInfo.size();
        suc -= lim_v;
    }

    if (suc < 1) {
    	suc = 1;
    }

    // 检查是否满足WRR算法的要求: 如果 mPreAll + suc 超出了此次轮转的分配限度，则扣除超出部分
    if (!firstSvr) {
        double preAllreq = (stat->mInfo.mPreAll + suc) * stat->mInfo.mLoadX;
        double dstReq = firstLoad - preAllreq;

        // 不满足WRR。扣除超出部分
        if (dstReq < 0) {
        	dstReq = -dstReq;
        	suc -= static_cast<int32_t> (static_cast<float>(dstReq) / stat->mInfo.mLoadX) + 1;
            preAllreq = (stat->mInfo.mPreAll + suc) * stat->mInfo.mLoadX;
            dstReq = firstLoad - preAllreq;
        }
    }

    if (suc < 1) {
    	suc = 1;
    }

    // 加上预分配数
    stat->mInfo.mPreAll += suc;
    stat->mInfo.mSPreAll += suc;
    //svr.mPre = suc;
    */

    stat->mInfo.mPreAll++;
    stat->mInfo.mSPreAll++;

    LOG_DEBUG(kSvrLog, "SvrQos::RouteCheck check success,GID(%d),XID(%d),HOST(%s),PORT(%d)", svr.mGid,svr.mXid,svr.mHost,svr.mPort);
    return SVR_RTN_ACCEPT;
}

int SvrQos::AddRouteNode(const struct SvrNet_t& svr, struct SvrStat_t* stat) {
    struct SvrKind_t kind(svr);
    kind.mRebuildTm = stat->mReqCfg.mRebuildTm;

    MultiMapNode_t*& table = mRouteTable[kind];
    if (!table) {
    	SAFE_NEW(MultiMapNode_t, table);
    }
    if (!table) {
    	return -1;
    }

    // 路由表中已有相关sid节点
    // 取优先级最低（负载最大）的那个作为新节点统计信息的默认值
    for (MultiMapNodeRIt_t it = table->rbegin(); it != table->rend(); ++it) {
        struct SvrNode_t& node = it->second;

        if (node.mStat->mInfo.mOffSide == 0) {	// 只计算本地节点
            // 初始化统计
            // 目的：解决被调扩容时，新加的服务器流量会瞬间爆增，然后在一个周期内恢复正常（正在运行时，突然新加一个服务器，
            // 如果mPreAll=0，GetRoute函数分配服务器时，会连续分配该新加的服务器）
            stat->mInfo = node.mStat->mInfo;
            
            // 初始化阈值
            stat->mReqCfg.mReqLimit = node.mStat->mReqCfg.mReqLimit;
            stat->mReqCfg.mReqMin = node.mStat->mReqCfg.mReqMin;
            stat->mReqCfg.mReqMax = node.mStat->mReqCfg.mReqMax;
            stat->mReqCfg.mReqErrMin = node.mStat->mReqCfg.mReqErrMin;
            stat->mReqCfg.mReqExtendRate = node.mStat->mReqCfg.mReqExtendRate;
            break;
        }
    }

    struct SvrNode_t node(svr, stat);
    table->insert(std::make_pair(node.mKey, node));

	LOG_DEBUG(kSvrLog, "SvrQos::AddRouteNode add SvrNet_t success, GID(%d),XID(%d),HOST(%s),PORT(%d),WEIGHT(%d),"
			"ReqLimit(%d),ReqAll(%d),ReqSuc(%d),ReqErrRet(%d),ReqErrTm(%d),LoadX(%f),PreAll(%d)",
			svr.mGid,svr.mXid,svr.mHost,svr.mPort,svr.mWeight,stat->mReqCfg.mReqLimit,stat->mInfo.mReqAll,
			stat->mInfo.mReqSuc,stat->mInfo.mReqErrRet,stat->mInfo.mReqErrTm,stat->mInfo.mLoadX,stat->mInfo.mReqAll);

	return 0;
}

int SvrQos::DeleteRouteNode(const struct SvrNet_t& svr) {
    struct SvrKind_t kind(svr);
    MapKindIt_t rtIt = mRouteTable.find(kind);
    MapNodeIt_t etIt = mErrTable.find(kind);

	if (rtIt == mRouteTable.end() && etIt == mErrTable.end()) {
		LOG_ERROR(kSvrLog, "SvrQos::DeleteRouteNode delete SvrNet_t failed(cannot find node from mRouteTable or mErrTable), GID(%d),XID(%d),HOST(%s),PORT(%d),WEIGHT(%d)",
				svr.mGid, svr.mXid, svr.mHost, svr.mPort, svr.mWeight);

        return -1;
    }

	// 节点路由
	if (rtIt != mRouteTable.end()) {
		MultiMapNode_t* table = rtIt->second;
        if (table != NULL) {
        	MultiMapNodeIt_t it = table->begin();
            while (it != table->end()) {
                if (it->second.mNet == svr) {
                	table->erase(it++);
                    if (table->empty()) {
                        mRouteTable.erase(rtIt);
                        SAFE_DELETE(table);
                    }
                    break;
                } else {
                    it++;
                }
            }
        } else {
    		LOG_DEBUG(kSvrLog, "SvrQos::DeleteRouteNode mRouteTable second(table) is null, GID(%d),XID(%d),HOST(%s),PORT(%d),WEIGHT(%d)",
    				svr.mGid, svr.mXid, svr.mHost, svr.mPort, svr.mWeight);
        }
	}

	// 错误路由
	if (etIt != mErrTable.end()) {
		ListNode_t* node = etIt->second;
		if (node != NULL) {
			ListNodeIt_t it = node->begin();
			while (it != node->end()) {
				if (it->mNet == svr) {
					node->erase(it++);
					if (node->empty()) {
                        mErrTable.erase(etIt);
                        SAFE_DELETE(node);
					}
					break;
				} else {
					it++;
				}
			}
		} else {
    		LOG_DEBUG(kSvrLog, "SvrQos::DeleteRouteNode mErrTable is null, GID(%d),XID(%d),HOST(%s),PORT(%d),WEIGHT(%d)",
    				svr.mGid, svr.mXid, svr.mHost, svr.mPort, svr.mWeight);
		}
	}
	return 0;
}

int SvrQos::ModifyRouteNode(const struct SvrNet_t& svr) {
    struct SvrKind_t kind(svr);
    MapKindIt_t rtIt = mRouteTable.find(kind);
    MapNodeIt_t etIt = mErrTable.find(kind);

	if (rtIt == mRouteTable.end() && etIt == mErrTable.end()) {
		LOG_ERROR(kSvrLog, "SvrQos::ModifyRouteNode modify SvrNet_t failed(cannot find node from mRouteTable or mErrTable), GID(%d),XID(%d),HOST(%s),PORT(%d),WEIGHT(%d)",
				svr.mGid, svr.mXid, svr.mHost, svr.mPort, svr.mWeight);
        return -1;
    }

	// 节点路由
	if (rtIt != mRouteTable.end()) {
		MultiMapNode_t* table = rtIt->second;
        if (table != NULL) {
        	MultiMapNodeIt_t it = table->begin();
            while (it != table->end()) {
                if (it->second.mNet == svr) {
                	struct SvrNet_t& oldsvr = it->second.mNet;
                	oldsvr = svr;
                	//oldsvr.mWeight = svr.mWeight;
					//oldsvr.mVersion = svr.mVersion;
					//oldsvr.mIdc = svr.mIdc;
					//memcpy(oldsvr.mName, svr.mName, kMaxName);
                    break;
                } else {
                    it++;
                }
            }
        } else {
    		LOG_DEBUG(kSvrLog, "SvrQos::ModifyRouteNode mRouteTable second(table) is null, GID(%d),XID(%d),HOST(%s),PORT(%d)",svr.mGid,svr.mXid,svr.mHost,svr.mPort);
        }
	}

	// 错误路由
	if (etIt != mErrTable.end()) {
		ListNode_t* node = etIt->second;
		if (node != NULL) {
			ListNodeIt_t it = node->begin();
			while (it != node->end()) {
				if (it->mNet == svr) {
                	struct SvrNet_t& oldsvr = it->mNet;
                	oldsvr = svr;
                	//oldsvr.mWeight = svr.mWeight;
					//oldsvr.mVersion = svr.mVersion;
					//oldsvr.mIdc = svr.mIdc;
					//memcpy(oldsvr.mName, svr.mName, kMaxName);
					break;
				} else {
					it++;
				}
			}
		} else {
    		LOG_DEBUG(kSvrLog, "SvrQos::ModifyRouteNode mRouteTable second(table) is null, GID(%d),XID(%d),HOST(%s),PORT(%d)",svr.mGid,svr.mXid,svr.mHost,svr.mPort);
		}
	}
	return 0;
}

const wStatus& SvrQos::CleanNode() {
    if (mMapReqSvr.size() > 0) {
    	struct SvrNet_t svr;
    	struct SvrStat_t* stat;
    	for (MapSvrIt_t mapReqIt = mMapReqSvr.begin(); mapReqIt != mMapReqSvr.end(); mapReqIt++) {
    		svr = mapReqIt->first;
    		stat = mapReqIt->second;
    		DeleteRouteNode(svr);
    		SAFE_DELETE(stat);
    	}
    	mMapReqSvr.clear();
    }
    return mStatus;
}
