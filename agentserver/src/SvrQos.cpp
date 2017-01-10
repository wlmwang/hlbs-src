
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

SvrQos::~SvrQos() {
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

bool SvrQos::IsVerChange(const struct SvrNet_t& svr) {
	mMutex.Lock();
	MapSvrIt_t mapReqIt = mMapReqSvr.find(svr);
	MapSvrIt_t mapEndIt = mMapReqSvr.end();
	mMutex.Unlock();
	if (mapReqIt != mapEndIt) {
		const struct SvrNet_t& kind = mapReqIt->first;
		if (kind.mVersion == svr.mVersion) {
			return false;
		}
	}
	return true;
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

	if (!LoadStatCfg(svr, stat).Ok()) {
		LOG_ERROR(kSvrLog, "SvrQos::AddNode add SvrNet_t failed(LoadStatCfg failed), GID(%d),XID(%d),HOST(%s),PORT(%d),WEIGHT(%d)",
				svr.mGid, svr.mXid, svr.mHost, svr.mPort, svr.mWeight);

		SAFE_DELETE(stat);
		return mStatus;
	}

	mMutex.Lock();
	LOG_DEBUG(kSvrLog, "SvrQos::AddNode add SvrNet_t success, GID(%d),XID(%d),HOST(%s),PORT(%d),WEIGHT(%d)",
			svr.mGid, svr.mXid, svr.mHost, svr.mPort, svr.mWeight);

	// 重建绝对时间
	stat->mReqCfg.mRebuildTm = mRebuildTm;
	misc::GetTimeofday(&stat->mInfo.mBuildTm);
	mMapReqSvr.insert(std::make_pair(svr, stat));
	mStatus = AddRouteNode(svr, stat);
	mMutex.Unlock();
	return mStatus;
}

const wStatus& SvrQos::ModifyNode(const struct SvrNet_t& svr) {
	LOG_DEBUG(kSvrLog, "SvrQos::ModifyNode modify SvrNet_t start, GID(%d),XID(%d),HOST(%s),PORT(%d),WEIGHT(%d)",
			svr.mGid, svr.mXid, svr.mHost, svr.mPort, svr.mWeight);

	if (svr.mWeight < 0) {
		LOG_ERROR(kSvrLog, "SvrQos::ModifyNode modify SvrNet_t failed(weight < 0), GID(%d),XID(%d),HOST(%s),PORT(%d),WEIGHT(%d)",
				svr.mGid, svr.mXid, svr.mHost, svr.mPort, svr.mWeight);

		return mStatus = wStatus::IOError("SvrQos::ModifyNode failed, the SvrNet_t(weight < 0) will be ignore", "");
	} else if (svr.mWeight == 0) {
		// 权重为0删除节点
		return DeleteNode(svr);
	} else {
		// 修改weight
		mMutex.Lock();
		MapSvrIt_t mapReqIt = mMapReqSvr.find(svr);
		struct SvrNet_t& oldsvr = const_cast<struct SvrNet_t&>(mapReqIt->first);
		oldsvr.mVersion = svr.mVersion;
		oldsvr.mWeight = svr.mWeight < kMaxWeight? svr.mWeight: kMaxWeight;
		mStatus = ModifyRouteNode(svr);
		mMutex.Unlock();
		return mStatus;
	}
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
    mStatus = DeleteRouteNode(svr);
    SAFE_DELETE(stat);
	mMutex.Unlock();
    return mStatus;
}

const wStatus& SvrQos::GetNodeAll(struct SvrNet_t buf[], int32_t* num) {
	LOG_DEBUG(kSvrLog, "SvrQos::GetNodeAll get all SvrNet_t start");

	mMutex.Lock();
	*num = 0;
	for (MapSvrIt_t mapReqIt = mMapReqSvr.begin(); mapReqIt != mMapReqSvr.end(); mapReqIt++) {
		buf[(*num)++] = mapReqIt->first;
	}
	mMutex.Unlock();
	return mStatus.Clear();
}

const wStatus& SvrQos::QueryNode(struct SvrNet_t& svr) {
	LOG_DEBUG(kSvrLog, "SvrQos::QueryNode query SvrNet_t start, GID(%d),XID(%d)", svr.mGid, svr.mXid);

	mMutex.Lock();
	mStatus = GetRouteNode(svr);
	mMutex.Unlock();

	LOG_DEBUG(kSvrLog, "SvrQos::QueryNode query SvrNet_t end, GID(%d),XID(%d),HOST(%s),PORT(%d),WEIGHT(%d)",
			svr.mGid, svr.mXid, svr.mHost, svr.mPort, svr.mWeight);
	return mStatus;
}

const wStatus& SvrQos::CallerNode(const struct SvrCaller_t& caller) {
	LOG_DEBUG(kSvrLog, "SvrQos::CallerNode report SvrCaller_t start, GID(%d),XID(%d),HOST(%s),PORT(%d),WEIGHT(%d),ReqRet(%d),ReqCount(%d),ReqUsetimeUsec(%lld)",
			caller.mCalledGid, caller.mCalledXid, caller.mHost, caller.mPort, caller.mReqRet, caller.mReqCount, caller.mReqUsetimeUsec);

	mMutex.Lock();
	mStatus = ReportNode(caller);
	mMutex.Unlock();

	// 调试信息
	SvrKind_t kind;
	kind.mGid = caller.mCalledGid;
	kind.mXid = caller.mCalledXid;

	MapKindIt_t mapIt = mRouteTable.find(kind);
	if (mapIt != mRouteTable.end()) {
		MultiMapNode_t* mapNode = mapIt->second;
		for (MultiMapNodeIt_t it = mapNode->begin(); it != mapNode->end(); it++) {
			struct SvrNet_t svr = it->second.mNet;
			struct SvrStat_t stat = *it->second.mStat;

			LOG_DEBUG(kSvrLog, "SvrQos::CallerNode RouteTable, LOADX(%f),GID(%d),XID(%d),HOST(%s),PORT(%d),WEIGHT(%d),"
					"ReqLimit(%d),ReqMax(%d),ReqMin(%d),ReqCount(%d),ReqErrMin(%f),ReqExtendRate(%f)"
					"ReqAll(%d),ReqRej(%d),ReqSuc(%d),ReqErrRet(%d),ReqErrTm(%d),LoadX(%f),OkLoad(%f),DelayLoad(%f),AvgTm(%d),AvgErrRate(%f)",
					it->first, svr.mGid, svr.mXid, svr.mHost, svr.mPort, svr.mWeight,
					stat.mReqCfg.mReqLimit, stat.mReqCfg.mReqMax,stat.mReqCfg.mReqMin, stat.mReqCfg.mReqCount, stat.mReqCfg.mReqErrMin, stat.mReqCfg.mReqExtendRate,
					stat.mInfo.mReqAll, stat.mInfo.mReqRej, stat.mInfo.mReqSuc, stat.mInfo.mReqErrRet, stat.mInfo.mReqErrTm,
					stat.mInfo.mLoadX, stat.mInfo.mOkLoad, stat.mInfo.mDelayLoad, stat.mInfo.mAvgTm, stat.mInfo.mAvgErrRate);
		}
	}
    return mStatus;
}

const wStatus& SvrQos::ReportNode(const struct SvrCaller_t& caller) {
	if (caller.mCalledGid <= 0 || caller.mCalledXid <= 0 || caller.mPort <= 0 || caller.mHost[0] == 0) {
		LOG_ERROR(kSvrLog, "SvrQos::ReportNode report failed(caller data illegal), GID(%d),XID(%d),HOST(%s),PORT(%d)",
				caller.mCalledGid, caller.mCalledXid, caller.mHost, caller.mPort);

		return mStatus = wStatus::IOError("SvrQos::ReportNode failed, caller data illegal", "");
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

		return mStatus = wStatus::IOError("SvrQos::ReportNode report failed, cannot find caller node from MapReqSvr", "");
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
    mStatus = RebuildRoute(kind);
    return mStatus;
}

const wStatus& SvrQos::LoadStatCfg(const struct SvrNet_t& svr, struct SvrStat_t* stat) {
	stat->mInfo.InitInfo(svr);
	stat->mReqCfg = mReqCfg;
	return mStatus.Clear();
}

const wStatus& SvrQos::GetRouteNode(struct SvrNet_t& svr) {
	if (svr.mGid <= 0 || svr.mXid <= 0) {
		LOG_ERROR(kSvrLog, "SvrQos::GetRouteNode get failed(the SvrNet_t invalid), GID(%d),XID(%d),HOST(%s),PORT(%d),WEIGHT(%d)",
				svr.mGid, svr.mXid, svr.mHost, svr.mPort, svr.mWeight);

		return mStatus = wStatus::IOError("SvrQos::GetRouteNode get failed, the SvrNet_t invalid", "");
	}

	struct SvrKind_t kind(svr);

	// 重建该路由
	RebuildRoute(kind);

	MapKindIt_t rtIt = mRouteTable.find(kind);
	if (rtIt == mRouteTable.end()) {

		LOG_ERROR(kSvrLog, "SvrQos::GetRouteNode get failed(the SvrNet_t not exists from mRouteTable), GID(%d),XID(%d),HOST(%s),PORT(%d),WEIGHT(%d)",
				svr.mGid, svr.mXid, svr.mHost, svr.mPort, svr.mWeight);
		// todo
		// 路由不存在
		// 反向注册路由
		return mStatus;
    }

	struct SvrKind_t& stKind = const_cast<struct SvrKind_t&>(rtIt->first);
    stKind.mAccess64tm = misc::GetTimeofday();

	MultiMapNode_t* pTable = rtIt->second;
	if (pTable == NULL || pTable->empty()) {
		LOG_ERROR(kSvrLog, "SvrQos::GetRouteNode get failed(the SvrNet_t not exists(empty table)), GID(%d),XID(%d),HOST(%s),PORT(%d),WEIGHT(%d)",
				svr.mGid, svr.mXid, svr.mHost, svr.mPort, svr.mWeight);

		return mStatus = wStatus::IOError("SvrQos::GetRouteNode failed, empty table", "");
	}

    // 已分配到第几个路由
	int32_t iIndex = stKind.mPindex;
	if (iIndex >= static_cast<int32_t>(pTable->size())) {
		stKind.mPindex = iIndex = 0;
	}

	MultiMapNodeIt_t it = pTable->begin();

	// 此次WRR轮转的负载分配限度（首节点的 mKey = mLoadX 是最小值）
	double firstReq = it->second.mKey * it->second.mStat->mInfo.mPreAll;
	bool firstSvr = false;

	std::advance(it, iIndex);

	if (iIndex != 0) {
		double dCurAdjReq = it->second.mKey * it->second.mStat->mInfo.mPreAll;

		// 此处采用round_robin的经典算法，获得目标路由
		// 如果不是第一个路由,  获得比第一个路由的负载更低的路由
		while (dCurAdjReq >= firstReq) {
			do {
				++iIndex;
				it = pTable->begin();
				if (iIndex >= static_cast<int32_t>(pTable->size())) {
					stKind.mPindex = iIndex = 0;
					break;
				}
				std::advance(it, iIndex);
			} while (it->second.mStat->mInfo.mOffSide == 1);

			if (iIndex == 0) {
				break;
			}
			dCurAdjReq = it->second.mKey * it->second.mStat->mInfo.mPreAll;
			stKind.mPindex = iIndex;
		}
	}

	// 未找到合适节点 或 第一个节点即为合适节点
	if (iIndex == 0) {
		firstSvr = true;
	}

	// 已经获得了预选路由即host,port
	memcpy(svr.mHost, it->second.mNet.mHost, sizeof(svr.mHost));
	svr.mPort = it->second.mNet.mPort;
	svr.mWeight = it->second.mNet.mWeight;

    // 检测分配路由，如果有路由分配产生，则更新相关统计计数
    int ret;
	while ((ret = RouteCheck(it->second.mStat, svr, firstReq, firstSvr)) != 0) {
		iIndex++;
		firstSvr = false;
		if (iIndex >= static_cast<int32_t>(pTable->size())) {
			iIndex = 0;
			firstSvr = true;
		}

		if (iIndex == stKind.mPindex) {
			// 一个轮回
			it = pTable->begin();
			std::advance(it, iIndex);

            if (ret == -2) {
            	memcpy(svr.mHost, it->second.mNet.mHost, sizeof(svr.mHost));
            	svr.mPort = it->second.mNet.mPort;
            	svr.mWeight = it->second.mNet.mWeight;
            	break;
            } else {
            	// 整体过载
                it->second.mStat->mInfo.mReqRej++;
                it->second.mStat->mInfo.mSReqRej++;

        		LOG_ERROR(kSvrLog, "SvrQos::GetRouteNode get failed(all SvrNet_t overload), GID(%d),XID(%d),HOST(%s),PORT(%d),WEIGHT(%d),ReqRej(%d)",
        				svr.mGid, svr.mXid, svr.mHost, svr.mPort, svr.mWeight, it->second.mStat->mInfo.mReqRej);

                return mStatus = wStatus::IOError("SvrQos::GetRouteNode get failed, all SvrNet_t Overload", "");
            }
		}

		it = pTable->begin();
		std::advance(it, iIndex);
		memcpy(svr.mHost, it->second.mNet.mHost, sizeof(svr.mHost));
		svr.mPort = it->second.mNet.mPort;
		svr.mWeight = it->second.mNet.mWeight;
	}

	// 过期时间
	int32_t tm = time(NULL);
	svr.mExpired = tm + stKind.mRebuildTm - (tm - stKind.mPtm);

	// 如果是第一个路由, 获取负载更低的路由（探测下一个路由）
	if (pTable->begin() == it) {
		it++;
		if (it != pTable->end() && (it->second.mStat->mInfo.mPreAll * it->second.mKey < firstReq)) {
			if ((iIndex + 1) < static_cast<int32_t>(pTable->size())) {
				iIndex++;
			} else {
				iIndex = 0;
			}
			stKind.mPindex = iIndex;
		}
		return mStatus.Clear();
	}

    // 如果不是第一个路由,  若负载比第一个路由大，滚动到下一个路由
	firstReq = pTable->begin()->second.mKey * pTable->begin()->second.mStat->mInfo.mPreAll;
	if (it->second.mStat->mInfo.mPreAll * it->second.mKey >= firstReq) {
		if ((iIndex + 1) < static_cast<int32_t>(pTable->size())) {
			stKind.mPindex = iIndex + 1;
		} else {
			stKind.mPindex = 0;
		}
	}

	LOG_DEBUG(kSvrLog, "SvrQos::GetRouteNode get success, GID(%d),XID(%d),HOST(%s),PORT(%d),WEIGHT(%d),index(%d)",
			svr.mGid, svr.mXid, svr.mHost, svr.mPort, svr.mWeight, stKind.mPindex);

	return mStatus.Clear();
}

const wStatus& SvrQos::RebuildRoute(struct SvrKind_t& kind, bool force) {
	MapKindIt_t rtIt = mRouteTable.find(kind);
	if (rtIt == mRouteTable.end()) {
		// 全部过载
		mAllReqMin = true;

		MultiMapNode_t* table;
		SAFE_NEW(MultiMapNode_t, table);
        // 重建宕机路由
        if (RebuildErrRoute(kind, table, 1, 1, 1).Ok() && !table->empty()) {
        	// 有ping测试通过路由
        	kind.mPindex = 0;
            mRouteTable.insert(std::make_pair(kind, table));
        } else {
            SAFE_DELETE(table);
        }
        return mStatus;
	}

	MultiMapNode_t* table = rtIt->second;
	if (table == NULL || table->empty()) {
		LOG_ERROR(kSvrLog, "SvrQos::RebuildRoute rebuild failed(the kind nothing table), GID(%d),XID(%d)", kind.mGid, kind.mXid);

        return mStatus;
	}
	struct SvrKind_t& stKind = const_cast<struct SvrKind_t&>(rtIt->first);

	int32_t tm = static_cast<int32_t>(time(NULL));
	if ((tm - stKind.mPtm) > 2 * stKind.mRebuildTm || (stKind.mPtm - tm) > 2 * stKind.mRebuildTm) {
		// 防止时间跳变（保证间隔二周期内才更新路由）
		stKind.mPtm = tm;
	}
    if (!force && (tm - stKind.mPtm < stKind.mRebuildTm)) {
    	// 如果非强制更新，并且还没到路由rebuild时间直接返回，默认一分钟rebuild一次
    	return mStatus;
    }
    // rebuild时间
    kind.mPtm = stKind.mPtm = tm;

    LOG_DEBUG(kSvrLog, "SvrQos::RebuildRoute rebuild start, GID(%d),XID(%d)", kind.mGid, kind.mXid);

    bool errRateBig = true;  // 所有被调都过载标志
    int32_t routeTotalReq = 0; // 请求总数

    uint64_t lowDelay = kDelayMax;	// 路由最低延时时间
    float heightSucRate = 0.0;		// 路由最高成功率
    float totalErrRate = 0.0;		// 路由失败率总和
    float highWeight = 0.0;			// 路由最高权重值
	float cfgErrRate = 0.0;			// 访问量控制错误率最小值

    // 统计数量
    for (MultiMapNodeIt_t it = table->begin(); it != table->end(); it++) {
    	struct SvrInfo_t &info = it->second.mStat->mInfo;

    	int32_t reqAll = info.mReqAll;
    	int32_t reqRej = info.mReqRej;
    	int32_t reqSuc = info.mReqSuc;
    	int32_t reqErrRet = info.mReqErrRet;
    	int32_t reqErrTm = info.mReqErrTm;

    	cfgErrRate = it->second.mStat->mReqCfg.mReqErrMin;
    	routeTotalReq += reqAll - reqRej;
    	if (reqAll < reqSuc + reqErrRet + reqErrTm) {
    		// 重置总请求数
    		reqAll = reqSuc + reqErrRet + reqErrTm;
    	}

    	// 门限控制、计算成功率、平均延时值
    	RouteNodeRebuild(it->second.mNet, it->second.mStat);

    	// 路由实际错误率
    	float nodeErrRate = 1 - info.mOkRate;
    	if (it->second.mStat->mReqCfg.mReqErrMin > nodeErrRate - info.mAvgErrRate /* 有效错误率 = 实际错误率 - 平均错误率 */) {
    		// 重置所有被调都过载标志（接触非压力故障）
    		errRateBig = false;
    	}

    	if (nodeErrRate < it->second.mStat->mReqCfg.mReqErrMin)	{
    		// 节点正常，清除连续过载标志
    		stKind.mPsubCycCount = 0;
    		stKind.mPtotalErrRate= 0;
    	} else {
    		// 节点过载
            char alartInfo[512];
            ::snprintf(alartInfo, sizeof(alartInfo) -1 , "SvrQos::RebuildRoute one SvrNet_t ready to overload,GID(%d),XID(%d),HOST(%s),PORT(%d),"
            		"ReqLimit(%d) ReqAll(%d) ReqSuc(%d) ReqRej(%d) reqErrRet(%d) reqErrTm(%d) server SvrNet_t errRate(%f)(%f)  > configure ERR_RATE(%f)",
					stKind.mGid, stKind.mXid, it->second.mNet.mHost, it->second.mNet.mPort,
					it->second.mStat->mReqCfg.mReqLimit, reqAll, reqSuc, reqRej, reqErrRet, reqErrTm, nodeErrRate, info.mAvgErrRate, cfgErrRate);

    		LOG_ERROR(kSvrLog, alartInfo);
    		// SendMobileMsg();
    	}

    	misc::GetTimeofday(&info.mBuildTm);

    	// 重置权重值
    	if (it->second.mNet.mWeight == 0) {
    		it->second.mNet.mWeight = kInitWeight;
    	}

    	totalErrRate += nodeErrRate; // 路由失败率总和
    	heightSucRate = heightSucRate >= info.mOkRate? heightSucRate: info.mOkRate;	// 路由最高成功率
    	lowDelay = lowDelay >= info.mAvgTm? info.mAvgTm: lowDelay; // 路由最低延时时间
    	highWeight = highWeight >= it->second.mNet.mWeight? highWeight: it->second.mNet.mWeight; // 路由最高权重值
    }
    if (lowDelay <= 0) {
    	lowDelay = 1;
    }

    // 全部节点都过载（非压力故障）
    if (errRateBig) {
    	// 所有节点过载平均错误率
    	float avgErrRate = totalErrRate / table->size();
    	stKind.mPtotalErrRate += avgErrRate;
    	stKind.mPsubCycCount++;

    	// 该类kind下所有节点过载平均错误率
    	mAvgErrRate = stKind.mPtotalErrRate / stKind.mPsubCycCount;

    	char alartInfo[512];
        ::snprintf(alartInfo, sizeof(alartInfo) -1 , "SvrQos::RebuildRoute all SvrNet_t ready to overload,GID(%d),XID(%d),"
        		"avgErrRate(%f) server all SvrNet_t errRate > configure ERR_RATE(%f)",
				stKind.mGid, stKind.mXid, avgErrRate, cfgErrRate);

        LOG_ERROR(kSvrLog, alartInfo);
	    // SendMobileMsg();
    }

    // 更新宕机请求数
    MapNodeIt_t reIt = mErrTable.find(kind);
	if (reIt != mErrTable.end()) {
		for (ListNodeIt_t eit = reIt->second->begin(); eit != reIt->second->end(); eit++) {
			// 累积宕机的服务自宕机开始，该kind下的所有节点请求数
			// 此请求数用来作为判断是否进行宕机恢复探测的依据
			eit->mReqAllAfterDown += routeTotalReq;
		}
	}

	mAllReqMin = true;

	MultiMapNode_t* newTable;
	SAFE_NEW(MultiMapNode_t, newTable);

	// 最大负载（最低权限）
	float bestLowPri = 1.0;
	// 最小成功率
    float bestLowSucRate = 1;
    // 最大延时值
    uint32_t bestBigDelay = 1;

	// 计算负载
	for (MultiMapNodeIt_t it = table->begin(); it != table->end(); it++) {
		struct SvrStat_t* pStat = it->second.mStat;
		struct SvrInfo_t& info = pStat->mInfo;

        cfgErrRate = pStat->mReqCfg.mReqErrMin;

        // 成功率负载
        if (info.mOkRate > 0) {
        	info.mOkLoad = heightSucRate / info.mOkRate;
        } else {
        	info.mOkLoad = kOkLoadMax;
        }
        info.mOkLoad = info.mOkLoad >= 1 ? info.mOkLoad : 1;	// 成功率负载
        info.mDelayLoad = static_cast<float>(info.mAvgTm) / static_cast<float>(lowDelay);	// 延时率负载
        float weightLoad = static_cast<float>(highWeight) / static_cast<float>(it->second.mNet.mWeight);	// 权重因子

        // 系统配置的负载因子
        if (mRateWeight == mDelayWeight) {
        	info.mLoadX = info.mDelayLoad * info.mOkLoad;
        } else {
        	info.mLoadX = info.mDelayLoad*mDelayWeight + info.mOkLoad*mRateWeight;
        }
        info.mLoadX *= weightLoad;

        if (info.mOffSide == 0) {
        	// 只计算同城就近接入节点
        	bestLowPri = bestLowPri >= info.mLoadX? bestLowPri: info.mLoadX;
        }

        // 恢复路由 || 宕机路由
        // set mIsDetecting = false
        struct SvrNode_t node(it->second.mNet, pStat);
        int32_t reqLimit = pStat->mReqCfg.mReqLimit <= 0 ? (pStat->mReqCfg.mReqMin + 1) : pStat->mReqCfg.mReqLimit;
        if (reqLimit > pStat->mReqCfg.mReqMin) {
        	if (info.mOffSide == 0) {
        		//只计算同城就近接入节点
            	bestLowSucRate = bestLowSucRate >= node.mStat->mInfo.mOkRate? node.mStat->mInfo.mOkRate: bestLowSucRate;
            	bestBigDelay = bestBigDelay >= node.mStat->mInfo.mAvgTm? bestBigDelay: node.mStat->mInfo.mAvgTm;
        	}
        	// 加入节点路由
        	newTable->insert(std::make_pair(node.mKey, node));
        	mAllReqMin = false;
        } else {
        	// 门限小于最低阈值，加入宕机列表
            AddErrRoute(kind, node);
        }
	}

	// 所有节点都过载
	if (mAllReqMin) {
        if (stKind.mPsubCycCount <= 0) {
        	stKind.mPsubCycCount = 1;
        }
        // 该类kind下所有节点过载平均错误率
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

	// 宕机探测
    RebuildErrRoute(kind, newTable, bestLowPri, bestLowSucRate, bestBigDelay);

    struct SvrKind_t newKind(stKind);
    newKind.mPindex = 0;

    // 重新添加
    mRouteTable.erase(rtIt);
    SAFE_DELETE(table);
    mRouteTable.insert(std::make_pair(newKind, newTable));
    return mStatus.Clear();
}

const wStatus& SvrQos::AddErrRoute(struct SvrKind_t& kind, struct SvrNode_t& node) {
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
    return mStatus.Clear();
}

const wStatus& SvrQos::RebuildErrRoute(struct SvrKind_t& kind, MultiMapNode_t* multiNode, float maxLoad, float lowOkRate, uint32_t bigDelay) {
	MapNodeIt_t reIt = mErrTable.find(kind);
    if (reIt == mErrTable.end()) {
        return mStatus.Clear();
    }

    ListNode_t* pErrRoute = reIt->second;
    if (pErrRoute != NULL && pErrRoute->empty()) {
        SAFE_DELETE(pErrRoute);
		mErrTable.erase(reIt);
		return mStatus.Clear();
	}

    // 全部过载，机器恢复后，负载设为最小
    if (mAllReqMin) {
    	maxLoad = 1;
	}

    // 宕机检测、故障恢复
    int32_t tm = static_cast<int32_t>(time(NULL)), ret, detectStat;
	bool del_flag;

    DetectThread::VecNode_t detectNodeadd, detectNodedel;

	ListNodeIt_t it = pErrRoute->begin();
	while (it != pErrRoute->end()) {
		// -1:网络探测失败 0:网络探测成功
		detectStat = -1;
		del_flag = false;

		// 故障探测：（mProbeBegin > 0 才打开网络层探测）
		//	1) 先进行网络层探测ping connect udp_icmp，网络探测成功后再用业务请求进行探测
		//	2) 机器宕机后该kind已经收到超过一定量的请求
		//	3) 机器宕机已经超过一定时间，需要进行一次探测
		struct DetectResult_t res;
		if (mDownCfg.mProbeBegin > 0 && tm > it->mStopTime + mDownCfg.mProbeBegin) {
			struct DetectNode_t detectNode(it->mNet.mHost, it->mNet.mPort, tm, tm + mDownCfg.mProbeNodeExpireTime);

			mDetectThread->GetDetectResult(detectNode, res, &ret);
			if (ret < 0) {
				// not found node
				detectNodeadd.push_back(detectNode);
			} else if (ret == 0) {
				// has detected

				if (res.mRc < 0) {
					// detect fail
					detectStat = -1;
				} else if (res.mDetectType == DETECT_TCP || res.mDetectType == DETECT_UDP) {
					// TCP or UDP success
					// 删除探测节点
					detectNodedel.push_back(detectNode);
					del_flag = true;
					detectStat = 0;
				} else {
					// 非法类型
					detectStat = -1;
				}
			}
		}

		// 在没达到恢复条件之前，如果网络探测成功，提前恢复
		if (!mAllReqMin && (tm - it->mStopTime < mDownCfg.mDownTimeTrigerProbe) && (it->mReqAllAfterDown < mDownCfg.mReqCountTrigerProbe)) {
			if (detectStat != 0) {
				// 探测不成功
				it++;
				continue;
			}
		}

		// 达到恢复条件，如果网络探测失败，推迟恢复
		if (mDownCfg.mProbeBegin > 0 && detectStat == -1) {
	        it++;
	        continue;
		}

		// 刚刚从故障机列表中放到正常机器列表，处在探测状态
		it->mIsDetecting = true;
		it->mKey = maxLoad;
		it->mStat->mReqCfg.mReqLimit = mDownCfg.mProbeTimes;

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
		multiNode->insert(std::make_pair(maxLoad, *it));
        pErrRoute->erase(it++);
	}

    if (pErrRoute->empty()) {
        SAFE_DELETE(pErrRoute);
		mErrTable.erase(reIt);
	}

    if (!detectNodeadd.empty()) {
    	mDetectThread->AddDetect(detectNodeadd);
    }

    if (!detectNodedel.empty()) {
    	mDetectThread->DelDetect(detectNodedel);
    }
	return mStatus.Clear();
}

int32_t SvrQos::GetAddCount(const struct SvrStat_t* stat, int32_t reqCount) {
    if (stat->mInfo.mLastErr) {
    	// 上周期存在过度扩张
        return std::max(((stat->mInfo.mLastAlarmReq - reqCount) * stat->mReqCfg.mReqExtendRate), static_cast<float>(stat->mReqCfg.mReqMin));
    } else {
    	// 上周期不存在过度扩张
        return std::max((stat->mReqCfg.mReqLimit * stat->mReqCfg.mReqExtendRate/stat->mInfo.mDelayLoad), static_cast<float>(stat->mReqCfg.mReqMin));
    }
}

const wStatus& SvrQos::RouteNodeRebuild(const struct SvrNet_t &svr, struct SvrStat_t* stat) {
	return ReqRebuild(svr, stat);
}

const wStatus& SvrQos::ReqRebuild(const struct SvrNet_t &svr, struct SvrStat_t* pSvrStat) {
	int32_t sucCount = pSvrStat->mInfo.mReqSuc;
	int32_t errCount = pSvrStat->mInfo.mReqErrRet + pSvrStat->mInfo.mReqErrTm;
    if (pSvrStat->mInfo.mReqAll < errCount + sucCount) {
    	// 防止核算错误
        pSvrStat->mInfo.mReqAll = errCount + sucCount;
    }

	// 预取数增加值
    // 预测的本周期的成功请求数（上个周期的成功请求数加上 上周期相对于上上个周期成功请求数的差值）在一个预取时间段（qos.xml中pre_time的值，默认为4秒）内的部分
	if (pSvrStat->mInfo.mIdle > 3) {
		pSvrStat->mInfo.mAddSuc = pSvrStat->mInfo.mReqSuc;
	} else if (pSvrStat->mInfo.mReqSuc > pSvrStat->mInfo.mLastReqSuc) {
		// 上个周期成功请求数 与 上上个周期成功请求数 的差值
		pSvrStat->mInfo.mAddSuc = pSvrStat->mInfo.mReqSuc - pSvrStat->mInfo.mLastReqSuc;
	} else {
		pSvrStat->mInfo.mAddSuc = 0;
	}

	// 空闲周期
	if (pSvrStat->mInfo.mReqSuc > 0) {
		pSvrStat->mInfo.mIdle = 0;
	} else {
		pSvrStat->mInfo.mIdle++;
	}

    // 保存上一周期数据
    pSvrStat->mInfo.mLastReqAll = pSvrStat->mInfo.mReqAll;
    pSvrStat->mInfo.mLastReqRej = pSvrStat->mInfo.mReqRej;
    pSvrStat->mInfo.mLastReqErrRet = pSvrStat->mInfo.mReqErrRet;
    pSvrStat->mInfo.mLastReqErrTm = pSvrStat->mInfo.mReqErrTm;
    pSvrStat->mInfo.mLastReqSuc = pSvrStat->mInfo.mReqSuc;
    if (pSvrStat->mInfo.mLastReqSuc < 0) {
		pSvrStat->mInfo.mLastReqSuc = 0;
	}

	// 上个周期无请求
    if (pSvrStat->mInfo.mReqAll <= 0 && (errCount + sucCount <= 0)) {
    	// 重置成功率为1
    	pSvrStat->mInfo.mOkRate = 1;

        // 重置请求信息
    	pSvrStat->Reset();
        return mStatus.Clear();
    }

    // 失败率
    float errRate = pSvrStat->mInfo.mReqAll > 0 ? static_cast<float>(errCount)/static_cast<float>(pSvrStat->mInfo.mReqAll) : 0;
    errRate = errRate < 1 ? errRate : 1;
    pSvrStat->mInfo.mOkRate = 1 - errRate;
    sucCount = sucCount > 0 ? sucCount : 0;

	// 核算节点平均延时值
    if (pSvrStat->mInfo.mReqSuc > 0) {
    	// 有成功的请求
        pSvrStat->mInfo.mAvgTm = pSvrStat->mInfo.mTotalUsec / pSvrStat->mInfo.mReqSuc;
    } else {
		// 只有失败的请求
        if (pSvrStat->mInfo.mReqAll > pSvrStat->mReqCfg.mReqMin) {
			// 失败请求数超过请求最小阈值，延时设置最大值
            pSvrStat->mInfo.mAvgTm = kDelayMax;	// 100s
        } else {
        	// 参照上周期平均延时核算本周期平均延时
            if (pSvrStat->mInfo.mAvgTm && pSvrStat->mInfo.mAvgTm < kDelayMax) {
            	// 延时增加一倍
                pSvrStat->mInfo.mAvgTm = 2 * pSvrStat->mInfo.mAvgTm;
            } else {
                pSvrStat->mInfo.mAvgTm = kDelayMax / 100;	// 1s
            }
		}
	}

	// 初始化门限值mReqLimit
	// 路由的过载判断逻辑修改为：路由的当前周期错误率超过配置的错误率阀值时才判断为过载
	if (pSvrStat->mReqCfg.mReqLimit <= 0) {
		if (pSvrStat->mInfo.mReqAll > pSvrStat->mReqCfg.mReqMin) {
			// 门限扩张为请求数
			pSvrStat->mReqCfg.mReqLimit = pSvrStat->mInfo.mReqAll;
		} else {
			// 门限最低值
			pSvrStat->mReqCfg.mReqLimit = pSvrStat->mReqCfg.mReqMin + 1;
		}
	}
	pSvrStat->mReqCfg.mReqCount = pSvrStat->mInfo.mReqAll;

    if (errRate <= pSvrStat->mReqCfg.mReqErrMin) {
    	// 节点错误率未超门限，重置节点平均失败率为0
    	pSvrStat->mInfo.mAvgErrRate = 0;
    }

    // 核算门限 mReqLimit
    if (pSvrStat->mReqCfg.mReqErrMin >=  errRate - pSvrStat->mInfo.mAvgErrRate /* 有效错误率 = 实际错误率 - 平均错误率 */) {
    	// 门限扩张
        if (pSvrStat->mInfo.mLastErr && sucCount > pSvrStat->mInfo.mLastAlarmReq) {
        	// 如果成功数大于上次过度扩张时的门限，则扩张门限不再有意义应按照配置文件中所定义的扩张因子进行扩张本周期的扩张值
            pSvrStat->mInfo.mLastErr = false;
        }

        // 门限期望值： pSvrStat->mInfo.mReqAll * (1 + mReqExtendRate)
        int32_t dest = pSvrStat->mInfo.mReqAll + static_cast<int>(pSvrStat->mInfo.mReqAll * mReqCfg.mReqExtendRate);

        // 扩张
        if (pSvrStat->mReqCfg.mReqLimit < dest) {
            pSvrStat->mReqCfg.mReqLimit += GetAddCount(pSvrStat, pSvrStat->mInfo.mReqAll);
            if (pSvrStat->mReqCfg.mReqLimit > dest) {
            	// 门限大于预定义的req_max，设置门限为req_max
                pSvrStat->mReqCfg.mReqLimit = dest;
            }
        }
    } else {
		// 门限收缩
		if (pSvrStat->mInfo.mOkRate > 0) {
			// 有效错误率
			errRate -= pSvrStat->mInfo.mAvgErrRate;
		}

		// 直接收缩至成功数
		// 因为宕机时直接缩到成功数，可能会使得下个周期的门限比其他机器的大
		// 所以门限值按照 req_limit 和req_count中较小的一个进行收缩
		if (pSvrStat->mInfo.mReqAll > pSvrStat->mReqCfg.mReqLimit) {
			pSvrStat->mReqCfg.mReqLimit = pSvrStat->mReqCfg.mReqLimit - static_cast<int32_t>(pSvrStat->mReqCfg.mReqLimit * errRate);
		} else {
			pSvrStat->mReqCfg.mReqLimit = pSvrStat->mInfo.mReqAll - static_cast<int32_t>(pSvrStat->mInfo.mReqAll * errRate);
		}

		// 是否过度扩张，为后续调整做参考
		if (pSvrStat->mInfo.mReqAll <= pSvrStat->mReqCfg.mReqMin) {
			pSvrStat->mInfo.mLastErr = false;
		} else {
			pSvrStat->mInfo.mLastErr = true;
		}
		pSvrStat->mInfo.mLastAlarmReq = pSvrStat->mInfo.mReqAll;
		pSvrStat->mInfo.mLastAlarmSucReq = sucCount;
	}

    // 宕机 "嫌疑" 判定：
    // mReqLimit 大于最小值+1，并且：
	// 	1) 连续失败次数累加达到设定值，则该机器有宕机嫌疑
	// 	2) 错误率大于一定的比例，则该机器有宕机嫌疑
	if (pSvrStat->mReqCfg.mReqLimit > (pSvrStat->mReqCfg.mReqMin + 1) &&
			((pSvrStat->mInfo.mContErrCount >= mDownCfg.mPossibleDownErrReq) || (errRate > mDownCfg.mPossbileDownErrRate))) {
		// 宕机门限设置最小值
		pSvrStat->mReqCfg.mReqLimit = pSvrStat->mReqCfg.mReqMin + 1;
	}

    // 硬限制：不可超出[REQ_MIN, REQ_MAX]的范围
    if (pSvrStat->mReqCfg.mReqLimit > pSvrStat->mReqCfg.mReqMax) {
        pSvrStat->mReqCfg.mReqLimit = pSvrStat->mReqCfg.mReqMax;
    }
    if (pSvrStat->mReqCfg.mReqLimit < pSvrStat->mReqCfg.mReqMin) {
    	// 设置为最小值（注意：不是最小值+1），会被加入宕机列表
        pSvrStat->mReqCfg.mReqLimit = pSvrStat->mReqCfg.mReqMin;
    }

    // 清除当前周期统计数据
    pSvrStat->Reset();
    return mStatus;
}

int32_t SvrQos::RouteCheck(struct SvrStat_t* stat, struct SvrNet_t& svr, double firstLoad, bool firstSvr) {
	if (stat->mInfo.mOffSide == 1) {
		return -2;
	}
	stat->mInfo.mReqAll++;
    stat->mInfo.mSReqAll++;

    // 路由预取数
    svr.mPre = 1;

    if (stat->mReqCfg.mReqLimit <= 0) {
    	// 请求数门限收缩到0值，被分配数翻倍
    	stat->mInfo.mPreAll++;
	    stat->mInfo.mSPreAll++;
        return 0;
    }

    float errRate = static_cast<float>(stat->mInfo.mReqErrRet + stat->mInfo.mReqErrTm) / static_cast<float>(stat->mInfo.mReqAll);

    if (stat->mInfo.mReqAll >= stat->mReqCfg.mReqLimit) {
    	// 当访问量超过门限
		if (errRate > stat->mReqCfg.mReqErrMin) {
			// 且当前周期的错误率超过配置的错误率阀值时才判断为过载

			stat->mInfo.mReqAll--;
		    stat->mInfo.mSReqAll--;
		    stat->mInfo.mPreAll++;

		    // 过载
	        LOG_ERROR(kSvrLog, "SvrQos::RouteCheck check failed(the SvrNet_t overload),GID(%d),XID(%d),HOST(%s),PORT(%d),",
	        		svr.mGid, svr.mXid, svr.mHost, svr.mPort);
            return -1;
        }
		stat->mInfo.mPreAll++;
		stat->mInfo.mSPreAll++;
        return 0;
    }

    if (!firstSvr) {
        // 不是第一个节点
    	double curAllReq = stat->mInfo.mPreAll * stat->mInfo.mLoadX;
        if (curAllReq >= firstLoad) {
        	// 当前负载高于第一个负载，被分配数翻倍。返回的路由预取数为1
        	stat->mInfo.mPreAll++;
			stat->mInfo.mSPreAll++;
	        return 0;
        }
	}

	if (errRate > (stat->mReqCfg.mReqErrMin + stat->mInfo.mAvgErrRate)) {
		// 若节点的有效错误率高于预设的最低错误率，被分配数翻倍。返回的路由预取数为1
		stat->mInfo.mPreAll++;
		stat->mInfo.mSPreAll++;
        return 0;
	}

	// 计算预取数

    // 初始值为预测的本周期的成功请求数
    // 上个周期的成功请求数加上 上个周期相对于 上上个周期成功请求数的差值，在一个预取时间段（qos.xml中PRE_TIME的值，默认为4秒）内的部分
    int lastSuc = stat->mInfo.mLastReqSuc + 1;
    int suc = mReqCfg.mPreTime * ((lastSuc + stat->mInfo.mAddSuc) / stat->mReqCfg.mRebuildTm);

	// 节点有效错误率
    float errate = 1 - stat->mInfo.mOkRate - stat->mInfo.mAvgErrRate;

    // 预取数收缩
    if (errate > 0 && errate < 1) {
    	suc -= static_cast<int32_t>(suc * errate);
    	if (stat->mInfo.mLoadX > 1) {
    		suc = static_cast<int32_t>(suc / stat->mInfo.mLoadX);
    	}
    }

    // 构建时间间隔多余120s
    if (time(NULL) - stat->mInfo.mBuildTm.tv_sec > 120) {
    	suc = 1;
    }

    // 已分配数 + 预取数 > mReqLimit
    // 则从mPre中扣除
    if ((stat->mInfo.mPreAll + suc) > stat->mReqCfg.mReqLimit) {
        int32_t limv = stat->mInfo.mPreAll  + suc  - stat->mReqCfg.mReqLimit;
        suc -= limv;
    }

    if (suc < 1) {
    	suc = 1;
    }

    // 检查是否满足WRR算法的要求: 如果 mPreAll + iSuc 超出了此次轮转的分配限度，则扣除超出部分
    if (!firstSvr) {
        double preAllreq = (stat->mInfo.mPreAll + suc) * stat->mInfo.mLoadX;
        double dstReq = firstLoad - preAllreq;
        if (dstReq < 0) {
        	// 扣除超出部分
        	dstReq = -dstReq;
        	suc -= static_cast<int32_t> (static_cast<float>(dstReq) / stat->mInfo.mLoadX) + 1;
            preAllreq = (stat->mInfo.mPreAll + suc) * stat->mInfo.mLoadX;
            dstReq = firstLoad - preAllreq;
        }
    }

    if (suc < 1) {
    	suc = 1;
    }

    stat->mInfo.mPreAll += suc;
    stat->mInfo.mSPreAll += suc;
    svr.mPre = suc;

    LOG_DEBUG(kSvrLog, "SvrQos::RouteCheck check success,GID(%d),XID(%d),HOST(%s),PORT(%d),PRE(%d)",
    		svr.mGid, svr.mXid, svr.mHost, svr.mPort, svr.mPre);
    return 0;
}

const wStatus& SvrQos::AddRouteNode(const struct SvrNet_t& svr, struct SvrStat_t* stat) {
    struct SvrKind_t kind(svr);
    kind.mRebuildTm = stat->mReqCfg.mRebuildTm;

    MultiMapNode_t*& table = mRouteTable[kind];
    if (table == NULL) {
    	SAFE_NEW(MultiMapNode_t, table);
    }

    // 路由表中已有相关kind（类型）节点，取优先级最低的那个作为新节点统计信息的默认值
    for (MultiMapNodeRIt_t it = table->rbegin(); it != table->rend(); ++it) {
        struct SvrNode_t& node = it->second;

        if (node.mStat->mInfo.mOffSide == 0) {
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
			svr.mGid, svr.mXid, svr.mHost, svr.mPort, svr.mWeight,stat->mReqCfg.mReqLimit,
			stat->mInfo.mReqAll,stat->mInfo.mReqSuc,stat->mInfo.mReqErrRet,stat->mInfo.mReqErrTm,stat->mInfo.mLoadX,stat->mInfo.mReqAll);
    return mStatus.Clear();
}

const wStatus& SvrQos::DeleteRouteNode(const struct SvrNet_t& svr) {
    struct SvrKind_t kind(svr);
    MapKindIt_t rtIt = mRouteTable.find(kind);
    MapNodeIt_t etIt = mErrTable.find(kind);

	if (rtIt == mRouteTable.end() && etIt == mErrTable.end()) {
		LOG_ERROR(kSvrLog, "SvrQos::DeleteRouteNode delete SvrNet_t failed(cannot find node from mRouteTable or mErrTable), GID(%d),XID(%d),HOST(%s),PORT(%d),WEIGHT(%d)",
				svr.mGid, svr.mXid, svr.mHost, svr.mPort, svr.mWeight);

        return mStatus = wStatus::IOError("SvrQos::DeleteRouteNode failed, cannot find the SvrNet from mRouteTable or mErrTable", "");
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
	return mStatus.Clear();
}

const wStatus& SvrQos::ModifyRouteNode(const struct SvrNet_t& svr) {
    struct SvrKind_t kind(svr);
    MapKindIt_t rtIt = mRouteTable.find(kind);
    MapNodeIt_t etIt = mErrTable.find(kind);

	if (rtIt == mRouteTable.end() && etIt == mErrTable.end()) {
		LOG_ERROR(kSvrLog, "SvrQos::ModifyRouteNode modify SvrNet_t failed(cannot find node from mRouteTable or mErrTable), GID(%d),XID(%d),HOST(%s),PORT(%d),WEIGHT(%d)",
				svr.mGid, svr.mXid, svr.mHost, svr.mPort, svr.mWeight);

        return mStatus = wStatus::IOError("SvrQos::ModifyRouteNode failed, cannot find the SvrNet from mRouteTable or mErrTable", "");
    }

	// 节点路由
	if (rtIt != mRouteTable.end()) {
		MultiMapNode_t* table = rtIt->second;
        if (table != NULL) {
        	MultiMapNodeIt_t it = table->begin();
            while (it != table->end()) {
                if (it->second.mNet == svr) {
                	struct SvrNet_t& oldsvr = it->second.mNet;
                	oldsvr.mWeight = svr.mWeight;
					oldsvr.mVersion = svr.mVersion;
                    break;
                } else {
                    it++;
                }
            }
        } else {
    		LOG_DEBUG(kSvrLog, "SvrQos::ModifyRouteNode mRouteTable second(table) is null, GID(%d),XID(%d),HOST(%s),PORT(%d),WEIGHT(%d)",
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
                	struct SvrNet_t& oldsvr = it->mNet;
                	oldsvr.mWeight = svr.mWeight;
					oldsvr.mVersion = svr.mVersion;
					break;
				} else {
					it++;
				}
			}
		} else {
    		LOG_DEBUG(kSvrLog, "SvrQos::ModifyRouteNode mRouteTable second(table) is null, GID(%d),XID(%d),HOST(%s),PORT(%d),WEIGHT(%d)",
    				svr.mGid, svr.mXid, svr.mHost, svr.mPort, svr.mWeight);
		}
	}
	return mStatus.Clear();
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
    return mStatus.Clear();
}
