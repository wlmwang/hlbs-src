
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include <vector>
#include <algorithm>
#include <cmath>
#include "wLogger.h"
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

	// 重建绝对时间
	stat->mReqCfg.mRebuildTm = mRebuildTm;
	misc::GetTimeofday(&stat->mInfo.mBuildTm);
	mMapReqSvr.insert(std::make_pair(svr, stat));

	LOG_DEBUG(kSvrLog, "SvrQos::AddNode add SvrNet_t success, GID(%d),XID(%d),HOST(%s),PORT(%d),WEIGHT(%d)",
			svr.mGid, svr.mXid, svr.mHost, svr.mPort, svr.mWeight);

	return AddRouteNode(svr, stat);
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
		LOG_ERROR(kSvrLog, "SvrQos::DeleteNode delete SvrNet_t failed(cannot find the SvrNet_t), GID(%d),XID(%d),HOST(%s),PORT(%d),WEIGHT(%d)",
				svr.mGid, svr.mXid, svr.mHost, svr.mPort, svr.mWeight);

        return mStatus = wStatus::IOError("SvrQos::DeleteNode failed, cannot find the SvrNet_t", "");
    }

    struct SvrStat_t* stat = mapReqIt->second;
    mMapReqSvr.erase(mapReqIt);
    DeleteRouteNode(svr);
    SAFE_DELETE(stat);

	LOG_DEBUG(kSvrLog, "SvrQos::DeleteNode delete SvrNet_t success, GID(%d),XID(%d),HOST(%s),PORT(%d),WEIGHT(%d)",
			svr.mGid, svr.mXid, svr.mHost, svr.mPort, svr.mWeight);

    return mStatus;
}

const wStatus& SvrQos::LoadStatCfg(const struct SvrNet_t& svr, struct SvrStat_t* stat) {
	stat->mInfo.InitInfo(svr);
	stat->mReqCfg = mReqCfg;
	return mStatus.Clear();
}

const wStatus& SvrQos::GetNodeAll(struct SvrNet_t buf[], int32_t* num, int32_t start, int32_t size) {
	LOG_DEBUG(kSvrLog, "SvrQos::GetNodeAll get all SvrNet_t start, [%d, %d]", start, size);

	*num = 0;
	MapSvrIt_t mapReqIt = mMapReqSvr.begin();
	for (std::advance(mapReqIt, start); mapReqIt != mMapReqSvr.end() && *num < size; mapReqIt++) {
		buf[(*num)++] = mapReqIt->first;
	}
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
