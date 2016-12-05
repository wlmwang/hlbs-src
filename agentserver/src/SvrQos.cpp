
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include <vector>
#include <algorithm>
#include <cmath>
#include "SvrQos.h"

SvrQos::~SvrQos() {
	CleanNode();
}

bool SvrQos::IsExistNode(const struct SvrNet_t& svr) {
	if (mMapReqSvr.find(svr) == mMapReqSvr.end()) {
		return false;
	}
	return true;
}

bool SvrQos::IsVerChange(const struct SvrNet_t& svr) {
	MapSvrIt_t mapReqIt = mMapReqSvr.find(svr);
	if (mapReqIt != mMapReqSvr.end()) {
		const struct SvrNet_t& kind = mapReqIt->first;
		if (kind.mVersion == svr.mVersion) {
			return false;
		}
	}
	return true;
}

const wStatus& SvrQos::SaveNode(const struct SvrNet_t& svr) {
	if (IsExistNode(svr)) {
		return ModifyNode(svr);
	}
	return AddNode(svr);
}

const wStatus& SvrQos::AddNode(const struct SvrNet_t& svr) {
	if (svr.mWeight <= 0) {
		return mStatus = wStatus::IOError("SvrQos::AddNode failed, the SvrNet(weight<=0) will be ignore", "");
	}

	struct SvrStat_t* stat;
	SAFE_NEW(SvrStat_t, stat);

	if (!LoadStatCfg(svr, stat).Ok()) {
		SAFE_DELETE(stat);
		return mStatus;
	}

	// 重建绝对时间
	misc::GetTimeofday(&stat->mInfo.mBuildTm);
	mMapReqSvr.insert(std::make_pair(svr, stat));
	return AddRouteNode(svr, stat);
}

const wStatus& SvrQos::ModifyNode(const struct SvrNet_t& svr) {
	if (svr.mWeight < 0) {
		return mStatus = wStatus::IOError("SvrQos::ModifyNode failed, the SvrNet(weight<0) will be ignore", "");
	} else if (svr.mWeight == 0) {
		// 权重为0删除节点
		return DeleteNode(svr);
	} else {
		// 修改weight
		MapSvrIt_t mapReqIt = mMapReqSvr.find(svr);
		struct SvrNet_t& oldsvr = const_cast<struct SvrNet_t&>(mapReqIt->first);
		oldsvr.mVersion = svr.mVersion;
		oldsvr.mWeight = svr.mWeight < kMaxWeight? svr.mWeight: kMaxWeight;
		return ModifyRouteNode(svr);
	}
}

const wStatus& SvrQos::DeleteNode(const struct SvrNet_t& svr) {
	MapSvrIt_t mapReqIt = mMapReqSvr.find(svr);
    if (mapReqIt == mMapReqSvr.end()) {
        return mStatus = wStatus::IOError("SvrQos::DeleteNode failed, cannot find the SvrNet", "");
    }

    struct SvrStat_t* stat = mapReqIt->second;
    SAFE_DELETE(stat);
    mMapReqSvr.erase(mapReqIt);
    return DeleteRouteNode(svr);
}

const wStatus& SvrQos::LoadStatCfg(const struct SvrNet_t& svr, struct SvrStat_t* stat) {
	stat->mInfo.InitInfo(svr);
	stat->mReqCfg = mReqCfg;
	return mStatus.Clear();
}

const wStatus& SvrQos::GetNodeAll(struct SvrNet_t buf[], int32_t* num) {
	*num = 0;
	for (MapSvrIt_t mapReqIt = mMapReqSvr.begin(); mapReqIt != mMapReqSvr.end(); mapReqIt++) {
		buf[(*num)++] = mapReqIt->first;
	}
	return mStatus.Clear();
}

const wStatus& SvrQos::QueryNode(struct SvrNet_t& svr) {
	return GetRouteNode(svr);
}

const wStatus& SvrQos::GetRouteNode(struct SvrNet_t& stSvr) {
	if (stSvr.mGid <= 0 || stSvr.mXid <= 0) {
		return mStatus = wStatus::IOError("SvrQos::GetRouteNode failed, the SvrNet_t invalid", "");
	}

	struct SvrKind_t kind(stSvr);

	// 重建该路由
	RebuildRoute(kind);

	MapKindIt_t rtIt = mRouteTable.find(kind);
	if (rtIt == mRouteTable.end()) {
		// 路由不存在
		// 反向注册路由 // TODO
		return mStatus = wStatus::IOError("SvrQos::GetRouteNode failed, cannot find router node", "");
    }

	MultiMapNode_t* pTable = rtIt->second;
	if (pTable == NULL || pTable->empty()) {
		SAFE_DELETE(pTable);
		mRouteTable.erase(rtIt);
		return mStatus = wStatus::IOError("SvrQos::GetRouteNode failed, empty table", "");
	}

    struct SvrKind_t& stKind = const_cast<struct SvrKind_t&>(rtIt->first);
    stKind.mAccess64tm = misc::GetTimeofday();

    // 已分配到第几个路由
	int32_t iIndex = stKind.mPindex;
	if (iIndex < 0 || iIndex >= static_cast<int32_t>(pTable->size())) {
		stKind.mPindex = iIndex = 0;
	}

	MultiMapNodeIt_t it = pTable->begin() , it1 = pTable->begin();

	// 此次WRR轮转的负载分配限度（首节点的 mKey = mLoadX 是最小值）
	double dFirstReq = it->second.mKey * it->second.mStat->mInfo.mPreAll;
	bool bFirstRt = false;

	std::advance(it, iIndex);

	if (iIndex != 0) {
		// 如果不是第一个路由，获得比第一个路由的负载更低的路由
		double dCurAdjReq = it->second.mKey * it->second.mStat->mInfo.mPreAll;

		// 此处采用round_robin的经典算法，获得目标路由
		while (dCurAdjReq >= dFirstReq) {
            if (++iIndex >= static_cast<int32_t>(pTable->size())) {
				it = pTable->begin();
                stKind.mPindex = iIndex = 0;
                break;
            }
            it++;
            stKind.mPindex = iIndex;
			dCurAdjReq = it->second.mKey * it->second.mStat->mInfo.mPreAll;
		}
	}

	// 未找到合适节点 或 第一个节点即为合适节点
	if (iIndex == 0) {
		bFirstRt = true;
	}

	// 已经获得了预选路由即host,port
	memcpy(stSvr.mHost, it->second.mNet.mHost, sizeof(stSvr.mHost));
	stSvr.mPort = it->second.mNet.mPort;
    stSvr.mWeight = it->second.mNet.mWeight;

    int iRet;
    // 检测分配路由，如果有路由分配产生，则更新相关统计计数
	while (!RouteCheck(it->second.mStat, stSvr, dFirstReq, bFirstRt).Ok()) {
		iIndex++;
		bFirstRt = false;
		if (iIndex >= static_cast<int32_t>(pTable->size())) {
			iIndex = 0;
			bFirstRt = true;
		}

		if (iIndex == stKind.mPindex) {
			// 一个轮回
			it = pTable->begin();
			std::advance(it, iIndex);

        	// 整体过载
            it->second.mStat->mInfo.mReqRej++;
            it->second.mStat->mInfo.mSReqRej++;

            return mStatus = wStatus::IOError("SvrQos::GetRouteNode failed, all Node Overload", "");
		}

		it = pTable->begin();
		std::advance(it, iIndex);
		memcpy(stSvr.mHost, it->second.mNet.mHost, sizeof(stSvr.mHost));
		stSvr.mPort = it->second.mNet.mPort;
	    stSvr.mWeight = it->second.mNet.mWeight;
	}

	int32_t tm = time(NULL);
	stSvr.mExpired = tm + stKind.mRebuildTm - (tm - stKind.mPtm);	// 过期时间

	// 如果是第一个路由, 获取负载更低的路由（探测下一个路由）
	if (pTable->begin() == it) {
		it++;
		if (it != pTable->end() && (it->second.mStat->mInfo.mPreAll * it->second.mKey < dFirstReq)) {
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
    dFirstReq = pTable->begin()->second.mKey * pTable->begin()->second.mStat->mInfo.mPreAll;
	int iPriCurReqSuc = it->second.mStat->mInfo.mPreAll;
	if (iPriCurReqSuc * it->second.mKey >= dFirstReq) {
		if ((iIndex + 1) < static_cast<int32_t>(pTable->size())) {
			stKind.mPindex = iIndex + 1;
		} else {
			stKind.mPindex = 0;
		}
	}
	return mStatus.Clear();
}

const wStatus& SvrQos::CallerNode(const struct SvrCaller_t& caller) {
	if (caller.mCalledGid <= 0 || caller.mCalledXid <= 0 || caller.mPort <= 0 || caller.mHost[0] == 0) {
		return mStatus = wStatus::IOError("SvrQos::CallerNode failed", "illegal caller data");
	}

	struct SvrNet_t svr;
	svr.mGid = caller.mCalledGid;
	svr.mXid = caller.mCalledXid;
	svr.mPort = caller.mPort;
	memcpy(svr.mHost, caller.mHost, strlen(caller.mHost)+1);

	int64_t usec = caller.mReqUsetimeUsec;
	if (caller.mReqUsetimeUsec <= 0) {
		usec = 1;
	}

	MapSvrIt_t mapReqIt = mMapReqSvr.find(svr);
    if (mapReqIt == mMapReqSvr.end()) {
		return mStatus = wStatus::IOError("SvrQos::CallerNode failed", "cannot find caller node");
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
    return RebuildRoute(kind);
}

const wStatus& SvrQos::RebuildRoute(struct SvrKind_t& kind, bool force) {
	MapKindIt_t rtIt = mRouteTable.find(kind);
	if (rtIt == mRouteTable.end()) {
		// 全部过载
		mAllReqMin = true;

		MultiMapNode_t* table;
		SAFE_NEW(MultiMapNode_t, table);
        // 重建宕机路由
        if (RebuildErrRoute(kind, table).Ok() && !table->empty()) {
        	// 有ping测试通过路由
        	kind.mPindex = 0;
            mRouteTable.insert(std::make_pair(kind, table));
        } else {
            SAFE_DELETE(table);
        }

        //wStatus::IOError("SvrQos::RebuildRoute failed, recovery kind's error multiple node", "");
        return mStatus.Clear();
	}

	MultiMapNode_t* table = rtIt->second;
	if (table == NULL || table->empty()) {
        SAFE_DELETE(table);
        mRouteTable.erase(rtIt);
        return mStatus = wStatus::IOError("SvrQos::RebuildRoute failed, cannot find kind's multiple node", "");
	}
	struct SvrKind_t& stKind = const_cast<struct SvrKind_t&>(rtIt->first);

	int32_t tm = static_cast<int32_t>(time(NULL));
	if ((tm - stKind.mPtm) > 2 * stKind.mRebuildTm || (stKind.mPtm - tm) > 2 * stKind.mRebuildTm) {
		// 防止时间跳变（保证间隔二周期内才更新路由）
		stKind.mPtm = tm;
	}
    if (!force && (tm - stKind.mPtm < stKind.mRebuildTm)) {
    	// 如果非强制更新，并且还没到路由rebuild时间直接返回，默认一分钟rebuild一次
    	//wStatus::IOError("SvrQos::RebuildRoute failed, rebuild time is not reach", "");
    	return mStatus.Clear();
    }
    // rebuild时间 TODO
    kind.mPtm = stKind.mPtm = tm;

    bool errRateBig = true;  // 所有被调都过载标志
    int32_t routeTotalReq = 0; // 请求总数

    uint64_t lowDelay = kDelayMax;	// 路由最低延时时间
    float heightSucRate = 0.0;		// 路由最高成功率
    float totalErrRate = 0.0;		// 路由失败率总和
    float avgErrRate = 0.0;			// 路由多个周期平均错误率
    float highWeight = 0.0;			// 路由最高权重值

    // 统计数量
    for (MultiMapNodeIt_t it = table->begin(); it != table->end(); it++) {
    	struct SvrInfo_t &info = it->second.mStat->mInfo;
    	misc::GetTimeofday(&info.mBuildTm);

    	int32_t reqAll = info.mReqAll;
    	int32_t reqRej = info.mReqRej;
    	int32_t reqSuc = info.mReqSuc;
    	int32_t reqErrRet = info.mReqErrRet;
    	int32_t reqErrTm = info.mReqErrTm;

    	routeTotalReq += reqAll - reqRej;
    	if (reqAll < reqSuc + reqErrRet + reqErrTm) {
    		// 重置总请求数
    		reqAll = reqSuc + reqErrRet + reqErrTm;
    	}

    	// 门限控制、计算平均延时值、预取数
    	RouteNodeRebuild(it->second.mNet, it->second.mStat);

    	avgErrRate = info.mAvgErrRate;
    	float nodeErrRate = 1 - info.mOkRate;// 路由实际错误率

    	if (it->second.mStat->mReqCfg.mReqErrMin > nodeErrRate - avgErrRate /* 有效错误率 = 实际错误率 - 平均错误率 */) {
    		// 重置所有被调都过载标志
    		errRateBig = false;
    	}

    	// 过载逻辑
    	if (nodeErrRate < it->second.mStat->mReqCfg.mReqErrMin)	{
    		// 清除连续过载标志
    		stKind.mPsubCycCount = 0;
    		stKind.mPtotalErrRate= 0;
    	} else {
    		// 节点过载   // 通知 TODO
    		wStatus::IOError("SvrQos::RebuildRoute Overload(one) error rate > configure ERR_RATE", "");
    	}

    	// 重置权重值
    	if (it->second.mNet.mWeight == 0) {
    		it->second.mNet.mWeight = kInitWeight;
    	}

    	totalErrRate += nodeErrRate;							// 路由失败率总和
    	heightSucRate = std::max(heightSucRate, info.mOkRate);	// 路由最高成功率
    	lowDelay = std::min(lowDelay, info.mAvgTm);				// 路由最低延时时间
    	highWeight = std::max(highWeight, it->second.mNet.mWeight);		// 路由最高权重值
    }
    if (lowDelay <= 0) {
    	lowDelay = 1;
    }

    // 全部节点都过载
    if (errRateBig) {
    	// 所有节点过载平均错误率  // 通知 TODO
    	avgErrRate = totalErrRate / table->size();
    	stKind.mPtotalErrRate += totalErrRate / table->size();
    	stKind.mPsubCycCount++;

    	// 该类kind下所有节点过载平均错误率
    	mAvgErrRate = stKind.mPtotalErrRate / stKind.mPsubCycCount;

	    wStatus::IOError("SvrQos::RebuildRoute Overload(all) error rate > configure ERR_RATE", "");
    }

    // 更新宕机请求数 TODO
    MapNodeIt_t reIt = mErrTable.find(kind);
	if (reIt != mErrTable.end()) {
		for (ListNodeIt_t eit = reIt->second->begin(); eit != reIt->second->end(); eit++) {
			// 累积宕机的服务自宕机开始，该类svr的所有请求数
			// 此请求数用来作为判断是否进行宕机恢复探测的依据
			eit->mReqAllAfterDown += routeTotalReq;
		}
	}

	mAllReqMin = true;
	MultiMapNode_t* newTable;
	SAFE_NEW(MultiMapNode_t, newTable);

	// 访问量控制错误率最小值
	float cfgErrRate = 0
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

        // 成功率负载
        if (info.mOkRate > 0) {
        	info.mOkLoad = heightSucRate / info.mOkRate;
        } else {
        	info.mOkLoad = kOkLoadMax;
        }
        info.mOkLoad = info.mOkLoad >= 1 ? info.mOkLoad : 1;	// 成功率负载
        info.mDelayLoad = static_cast<float>(info.mAvgTm) / lowDelay;	// 延时率负载
        float weightLoad = static_cast<float>(highWeight) / it->second.mNet.mWeight;	// 权重因子

        // 系统配置的负载因子
        if (mRateWeight == mDelayWeight) {
        	info.mLoadX = info.mDelayLoad * info.mOkLoad;
        } else {
        	info.mLoadX = info.mDelayLoad*mDelayWeight + info.mOkLoad*mRateWeight;
        }
        info.mLoadX *= weightLoad;

        bestLowPri = std::max(bestLowPri, info.mLoadX);

        // 宕机路由
        struct SvrNode_t node(it->second.mNet, pStat);
        int32_t reqLimit = pStat->mReqCfg.mReqLimit <= 0 ? (pStat->mReqCfg.mReqMin + 1) : pStat->mReqCfg.mReqLimit;
        if (reqLimit > pStat->mReqCfg.mReqMin) {
        	mAllReqMin = false;
        	bestLowSucRate = std::min(bestLowSucRate, node.mStat->mInfo.mOkRate);
        	bestBigDelay = std::max(bestBigDelay, node.mStat->mInfo.mAvgTm);

        	// 加入节点路由
        	newTable->insert(std::make_pair(node.mKey, node));
        } else {
        	// 门限小于最低阈值，加入宕机列表
            AddErrRoute(kind, node);
        }

		// 访问量控制错误率最小值
        cfgErrRate = pStat->mReqCfg.mReqErrMin;
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

	    wStatus::IOError("SvrQos::RebuildRoute Overload(all) req_limit < configure REQ_LIMIT", "");

        // 通知 TODO
	}

	// 宕机探测
    RebuildErrRoute(kind, newTable, bestLowPri, bestLowSucRate, bestBigDelay);

    // 删除该kind下所有路由信息（重新添加）
    mRouteTable.erase(rtIt);
    SAFE_DELETE(table);

    struct SvrKind_t newKind(stKind);
    newKind.mPindex = 0;
    if (newTable != NULL && !newTable->empty()) {
    	mRouteTable.insert(std::make_pair(newKind, newTable));
    }
    return mStatus.Clear();
}

const wStatus& SvrQos::AddErrRoute(struct SvrKind_t& stItem, struct SvrNode_t& stNode) {
	MapNodeIt_t reIt = mErrTable.find(stItem);

	ListNode_t* pErrRoute;
    if (reIt == mErrTable.end()) {
        SAFE_NEW(ListNode_t, pErrRoute);
        stItem.mRebuildTm = stNode.mStat->mReqCfg.mRebuildTm;
        mErrTable.insert(std::make_pair(stItem, pErrRoute));
    } else {
        pErrRoute = reIt->second;
	}

	// record down node
	stNode.mStopTime = static_cast<int32_t>(time(NULL));
	stNode.mReqAllAfterDown = 0;
    pErrRoute->push_back(stNode);
    return mStatus.Clear();
}

const wStatus& SvrQos::RebuildErrRoute(struct SvrKind_t& stItem, MultiMapNode_t* pSvrNode, float iPri, float fLowOkRate, uint32_t iBigDelay) {
	MapNodeIt_t reIt = mErrTable.find(stItem);
    if (reIt == mErrTable.end()) {
        //wStatus::IOError("SvrQos::RebuildErrRoute failed, kind's error multiple node", "");
        return mStatus.Clear();
    }

    ListNode_t* pErrRoute = reIt->second;
    if (pErrRoute == NULL || pErrRoute->empty()) {
        SAFE_DELETE(pErrRoute);
		mErrTable.erase(reIt);
		return mStatus = wStatus::IOError("SvrQos::RebuildErrRoute failed, cannot find kind's multiple node", "");
	}

    if (mAllReqMin) {
		iPri = 1;
	}

	std::vector<struct DetectNode_t> vDetectNodeadd, vDetectNodedel;

	int iCurTm = time(NULL);
	// 宕机检测、故障恢复
	int iRet = -1;
	int iDetectStat = -1;

	ListNodeIt_t it = pErrRoute->begin();
	while (it != pErrRoute->end()) {
		struct DetectResult_t stRes;
		// -2: 没root权限探测  -1:网络探测失败 0:网络探测成功
		iDetectStat = -1;

		//故障探测：
		//1)先进行网络层探测ping connect udp_icmp，网络探测成功后再用业务请求进行探测
		//2)机器宕机后该sid已经收到超过一定量的请求
		//3)机器宕机已经超过一定时间，需要进行一次探测

		//mProbeBegin>0 才打开网络层探测
		if (mDownCfg.mProbeBegin > 0 && iCurTm > it->mStopTime + mDownCfg.mProbeBegin) {
			struct DetectNode_t stDetectNode(it->mNet.mHost, it->mNet.mPort, iCurTm, iCurTm + mDownCfg.mProbeNodeExpireTime);
			iRet = mDetectThread->GetDetectResult(stDetectNode, stRes);

			LOG_ERROR(ELOG_KEY, "[svr] RebuildErrRoute detect ret=%d,code=%d,type=%d,gid=%d xid=%d host=%s port=%u after_stop_time=%d,reqall_after_down=%d,expire=%d,limit=%d",
				iRet, stRes.mRc, stRes.mDetectType, it->mNet.mGid,it->mNet.mXid,it->mNet.mHost, it->mNet.mPort,
				iCurTm - it->mStopTime, it->mReqAllAfterDown, stDetectNode.mExpireTime - stDetectNode.mCreateTime, it->mStat->mReqCfg.mReqLimit);

			if (iRet < 0) {
				// not found
				vDetectNodeadd.push_back(stDetectNode);
			} else if (iRet == 0) {
				if (stRes.mRc < 0) {
					if (stRes.mRc == NOT_PRI) {
						//没有root权限，无法raw socket进行探测
						iDetectStat = -2;
					} else {
						iDetectStat = -1;
					}
				} else if (stRes.mDetectType == DETECT_TCP || stRes.mDetectType == DETECT_UDP) {
					//tcp or udp success, other fail
					vDetectNodedel.push_back(stDetectNode);
					//bDelFlag = true;
					iDetectStat = 0;
				} else {
					//tcp or udp fail
					iDetectStat = -1;
				}
			}
		}

		// 在没达到恢复条件之前，如果网络探测成功，提前恢复
		if (!mAllReqMin && (iCurTm - it->mStopTime < mDownCfg.mDownTimeTrigerProbe) && (it->mReqAllAfterDown < mDownCfg.mReqCountTrigerProbe)) {
			if (iDetectStat != 0) {
				// 探测不成功
				it++;
				continue;
			}
		}

		// 达到恢复条件，如果网络探测失败，推迟恢复
		if ((mDownCfg.mProbeBegin > 0) && (-1 == iDetectStat)) {
	        it++;
	        continue;
		}

		it->mStat->mReqCfg.mReqLimit = mDownCfg.mProbeTimes;
		it->mIsDetecting = true;
		it->mKey = iPri;

		if (mAllReqMin) {
            it->mStat->mInfo.mAvgErrRate = mAvgErrRate;
            it->mStat->mInfo.mLoadX = 1;
            it->mStat->mInfo.mOkLoad = 1;
            it->mStat->mInfo.mDelayLoad = 1;
        } else {
            it->mStat->mInfo.mLoadX = iPri;
            it->mStat->mInfo.mOkRate = fLowOkRate;
            it->mStat->mInfo.mAvgTm = iBigDelay;
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
		//it->mStat->mInfo.mSReqErrTm = 0;

		LOG_ERROR(ELOG_KEY, "[svr] RebuildErrRoute failed(cannot find errroute routenode in list) gid(%d),xid(%d)",stItem.mGid,stItem.mXid);

        pSvrNode->insert(make_pair(iPri, *it));
        pErrRoute->erase(it++);
	}

    if (pErrRoute->empty()) {
        SAFE_DELETE(pErrRoute);
		mErrTable.erase(reIt);
	}

    if (!vDetectNodeadd.empty()) {
    	mDetectThread->AddDetect(vDetectNodeadd);
    }

    if (!vDetectNodedel.empty()) {
    	mDetectThread->DelDetect(vDetectNodedel);
    }

	return mStatus.Clear();
}

int32_t SvrQos::GetAddCount(const struct SvrStat_t* stat, int32_t reqCount) {
    if (stat->mInfo.mLastErr) {
    	// 上周期存在过度扩张
        return std::max(((stat->mInfo.mLastAlarmReq - reqCount) * stat->mReqCfg.mReqExtendRate), stat->mReqCfg.mReqMin);
    } else {
    	// 上周期不存在过度扩张
        return std::max((stat->mReqCfg.mReqLimit * stat->mReqCfg.mReqExtendRate/stat->mInfo.mDelayLoad), stat->mReqCfg.mReqMin);
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
    // 预测的本周期的成功请求数（上个周期的成功请求数加上个周期相对于上上个周期成功请求数的差值）在一个预取时间段（qos.xml中pre_time的值，默认为4秒）内的部分
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
    if (pSvrStat->mInfo.mReqAll <= 0 && errCount + sucCount <= 0) {
    	// 重置成功率为1
    	pSvrStat->mInfo.mOkRate = 1;

        // 重置请求信息
    	pSvrStat->Reset();
        return mStatus.Clear();
    }

    // 失败率
    float errRate = pSvrStat->mInfo.mReqAll > 0 ? static_cast<float>(errCount)/pSvrStat->mInfo.mReqAll : 0;
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
    return mStatus.Clear();
}

const wStatus& SvrQos::RouteCheck(struct SvrStat_t* pSvrStat, struct SvrNet_t& stNode, double dFirstReq, bool bFirstRt) {
    pSvrStat->mInfo.mReqAll++;
    pSvrStat->mInfo.mSReqAll++;

    // 路由预取数
    stNode.mPre = 1;

    if (pSvrStat->mReqCfg.mReqLimit <= 0) {
    	// 请求数门限收缩到0值，被分配数翻倍
	    pSvrStat->mInfo.mPreAll++;
	    pSvrStat->mInfo.mSPreAll++;

	    wStatus::IOError("SvrQos::RouteCheck Success, but it's mReqLimit<=0", "");
        return mStatus.Clear();
    }

    int iReqCount = pSvrStat->mInfo.mReqAll;
    int iErrCount = pSvrStat->mInfo.mReqErrRet + pSvrStat->mInfo.mReqErrTm;
    float fErrRate = static_cast<float>(iErrCount) / iReqCount;

    if (iReqCount >= pSvrStat->mReqCfg.mReqLimit) {
    	// 请求超过门限，被分配数翻倍
		if (fErrRate > pSvrStat->mReqCfg.mReqErrMin) {
			// 且当前周期的错误率超过配置的错误率阀值时才判断为过载

			// 当真正发生过载的时候才对选定的路由的拒绝数加1
			//pSvrStat->mInfo.mReqRej++;
			LOG_ERROR(ELOG_KEY, "[svr] Overload RouteCheck gid=%d xid=%d host:%s port:%d", stNode.mGid,stNode.mXid,stNode.mHost,stNode.mPort);

		    pSvrStat->mInfo.mReqAll--;
		    pSvrStat->mInfo.mSReqAll--;

		    pSvrStat->mInfo.mPreAll++;

		    // 过载
            return mStatus = wStatus::IOError("SvrQos::RouteCheck failed, the SvrNet overload", "");
        }
		pSvrStat->mInfo.mPreAll++;
        pSvrStat->mInfo.mSPreAll++;
        return mStatus.Clear();
    }

    if (!bFirstRt) {
        // 不是第一个节点
    	double dCurAllReq = pSvrStat->mInfo.mPreAll * pSvrStat->mInfo.mLoadX;
        if (dCurAllReq >= dFirstReq) {
        	// 当前负载高于第一个负载，被分配数翻倍
			pSvrStat->mInfo.mPreAll++;
	        pSvrStat->mInfo.mSPreAll++;
	        return mStatus.Clear();
        }
	}

	if (fErrRate > (pSvrStat->mReqCfg.mReqErrMin + pSvrStat->mInfo.mAvgErrRate)) {
		// 若节点的有效错误率高于预设的最低错误率，被分配数翻倍
		pSvrStat->mInfo.mPreAll++;
        pSvrStat->mInfo.mSPreAll++;
        return mStatus.Clear();
	}

	// 节点总体错误率
    float fErrate = 1 - pSvrStat->mInfo.mOkRate - pSvrStat->mInfo.mAvgErrRate;

    // 初始值为预测的本周期的成功请求数
    // 上个周期的成功请求数加上 上个周期相对于 上上个周期成功请求数的差值，在一个预取时间段（qos.xml中PRE_TIME的值，默认为4秒）内的部分
    int iLastSuc = pSvrStat->mInfo.mLastReqSuc + 1;
    int iLastInc = pSvrStat->mInfo.mAddSuc;
    int iSuc = mReqCfg.mPreTime * ((iLastSuc + iLastInc) / pSvrStat->mReqCfg.mRebuildTm);

    // 预取数收缩
    if (fErrate > 0 && fErrate < 1) {
    	iSuc -= static_cast<int32_t>(iSuc * fErrate);
    	if (pSvrStat->mInfo.mLoadX > 1) {
    		iSuc = static_cast<int32_t>(iSuc / pSvrStat->mInfo.mLoadX);
    	}
    }

    // 构建时间间隔多余120s
    if (time(NULL) - pSvrStat->mInfo.mBuildTm.tv_sec > 120) {
    	iSuc = 1;
    }

    // 已分配数+预取数 > mReqLimit
    if ((pSvrStat->mInfo.mPreAll + iSuc) > pSvrStat->mReqCfg.mReqLimit) {
        int32_t iLimV = pSvrStat->mInfo.mPreAll  + iSuc  - pSvrStat->mReqCfg.mReqLimit;
        iSuc -= iLimV;
    }

    if (iSuc < 1) iSuc = 1;

    // 检查是否满足WRR算法的要求: 如果 mPreAll + iSuc 超出了此次轮转的分配限度，则扣除超出部分
    if (!bFirstRt) {
        double dPreAllreq = (pSvrStat->mInfo.mPreAll + iSuc) * pSvrStat->mInfo.mLoadX; // fixme
        double dDstReq = dFirstReq - dPreAllreq;
        if (dDstReq < 0) {
        	// 扣除超出部分
            dDstReq = -dDstReq;
            iSuc = iSuc - static_cast<int32_t> ((float) dDstReq / pSvrStat->mInfo.mLoadX) + 1;
            dPreAllreq = (pSvrStat->mInfo.mPreAll + iSuc) * pSvrStat->mInfo.mLoadX;
            dDstReq = dFirstReq - dPreAllreq;
        }
    }

    if (iSuc < 1) iSuc = 1;

    pSvrStat->mInfo.mPreAll += iSuc;
    pSvrStat->mInfo.mSPreAll += iSuc;
    stNode.mPre = iSuc;

    return mStatus.Clear();
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

    struct SvrNode_t node(svr, stat);
    table->insert(std::make_pair(node.mKey, node));
    return mStatus.Clear();
}

const wStatus& SvrQos::DeleteRouteNode(const struct SvrNet_t& svr) {
    struct SvrKind_t kind(svr);
    MapKindIt_t rtIt = mRouteTable.find(kind);
    MapNodeIt_t etIt = mErrTable.find(kind);

	if (rtIt == mRouteTable.end() && etIt == mErrTable.end()) {
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
		}
	}
	return mStatus.Clear();
}

const wStatus& SvrQos::ModifyRouteNode(const struct SvrNet_t& svr) {
    struct SvrKind_t kind(svr);
    MapKindIt_t rtIt = mRouteTable.find(kind);
    MapNodeIt_t etIt = mErrTable.find(kind);

	if (rtIt == mRouteTable.end() && etIt == mErrTable.end()) {
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
		}
	}
	return mStatus.Clear();
}

// 清除所有节点
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

