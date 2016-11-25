
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
	MapSvrIt_t mapReqIt = mMapReqSvr.find(svr);
	if (mapReqIt == mMapReqSvr.end()) {
		return false;
	}
	return true;
}

bool SvrQos::IsVerChange(const struct SvrNet_t& svr) {
	MapSvrIt_t mapReqIt = mMapReqSvr.find(svr);
	if (mapReqIt != mMapReqSvr.end()) {
		struct SvrNet_t& kind = const_cast<struct SvrNet_t&> (mapReqIt->first);
		if (kind.mVersion != svr.mVersion) {
			return true;
		}
	}
	return false;
}

const wStatus& SvrQos::GetNodeAll(struct SvrNet_t buf[], int32_t* num) {
	*num = 0;
	for (MapSvrIt_t mapReqIt = mMapReqSvr.begin(); mapReqIt != mMapReqSvr.end(); mapReqIt++) {
		buf[(*num)++] = mapReqIt->first;
	}
	return mStatus.Clear();
}

const wStatus& SvrQos::SaveNode(const struct SvrNet_t& svr) {
	MapSvrIt_t mapReqIt = mMapReqSvr.find(svr);
	if (mapReqIt == mMapReqSvr.end()) {
		// 添加新svr节点
		return AddNode(svr);
	} else {
		return ModifyNode(svr);
	}
}

const wStatus& SvrQos::AddNode(const struct SvrNet_t& svr) {
	struct SvrStat_t* stat;
	SAFE_NEW(SvrStat_t, stat);
	if (stat == NULL) {
		return mStatus = wStatus::IOError("SvrQos::AddNode failed", "new SvrStat_t failed");
	}
	// 重建绝对时间
	misc::GetTimeofday(&stat->mInfo.mBuildTm);

	if (!LoadStatCfg(svr, stat).Ok()) {
		SAFE_DELETE(stat);
		return mStatus;
	}

	// 设置节点重建时间间隔 && 插入节点
	stat->mReqCfg.mRebuildTm = mRebuildTm;
	mMapReqSvr[svr] = stat;

	return AddRouteNode(svr, stat);
}

const wStatus& SvrQos::ModifyNode(const struct SvrNet_t& svr) {
	if (svr.mWeight < 0) {
		return mStatus = wStatus::IOError("SvrQos::ModifyNode failed", "weight should be >= 0");
	} else if (svr.mWeight == 0) {
		// 权重为0删除节点
		return DeleteNode(svr);
	} else {
		// 修改weight
		MapSvrIt_t mapReqIt = mMapReqSvr.find(svr);
		struct SvrNet_t& oldsvr = const_cast<struct SvrNet_t&>(mapReqIt->first);
		if (svr.mWeight != oldsvr.mWeight) {
			oldsvr.mWeight = svr.mWeight < kMaxWeight? svr.mWeight: kMaxWeight;
		}
	}
	return mStatus.Clear();
}

const wStatus& SvrQos::DeleteNode(const struct SvrNet_t& svr) {
	MapSvrIt_t mapReqIt = mMapReqSvr.find(svr);
    if (mapReqIt == mMapReqSvr.end()) {
        return mStatus = wStatus::IOError("SvrQos::DeleteNode failed", "cannot find node");
    }

    SvrStat_t* stat = mapReqIt->second;
    mMapReqSvr.erase(mapReqIt);
    SAFE_DELETE(stat);

    return DeleteRouteNode(svr);
}

const wStatus& SvrQos::LoadStatCfg(const struct SvrNet_t& svr, struct SvrStat_t* stat) {
	stat->mInfo.InitInfo(svr);
	stat->mReqCfg = mReqCfg;
	return mStatus.Clear();
}

const wStatus& SvrQos::AddRouteNode(const struct SvrNet_t& svr, struct SvrStat_t* stat) {
    struct SvrKind_t kind(svr);
    kind.mRebuildTm = stat->mReqCfg.mRebuildTm;

    MultiMapNode_t*& table = mRouteTable[kind];
    if (table == NULL) {
    	SAFE_NEW(MultiMapNode_t, table);
        if (table == NULL) {
        	return mStatus = wStatus::IOError("SvrQos::AddRouteNode failed", "new std::multimap<float, struct SvrNode_t> failed");
        }
    }

    // 路由表中已有相关节点，取优先级最低的那个作为新节点统计信息的默认值
    for (MultiMapNodeRIt_t it = table->rbegin(); it != table->rend(); ++it) {
        struct SvrNode_t& node = it->second;
        // 初始化阈值
        stat->mReqCfg.mReqLimit = node.mStat->mReqCfg.mReqLimit;
        stat->mReqCfg.mReqMin = node.mStat->mReqCfg.mReqMin;
        stat->mReqCfg.mReqMax = node.mStat->mReqCfg.mReqMax;
        stat->mReqCfg.mReqErrMin = node.mStat->mReqCfg.mReqErrMin;
        stat->mReqCfg.mReqExtendRate = node.mStat->mReqCfg.mReqExtendRate;

        // 初始化统计
        // 正在运行时，突然新加一个服务器，如果mPreAll=0，GetRoute函数分配服务器时，会连续分配该新加的服务器
        // 导致：被调扩容时，新加的服务器流量会瞬间爆增，然后在一个周期内恢复正常
        stat->mInfo = node.mStat->mInfo;
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
        return mStatus = wStatus::IOError("SvrQos::DeleteRouteNode failed", "cannot find svr from mRouteTable or mErrTable");
    }

	// 路由
	if (rtIt != mRouteTable.end()) {
		MultiMapNode_t* pTable = rtIt->second;
        if (pTable != NULL) {
        	MultiMapNodeIt_t it = pTable->begin();
            while (it != pTable->end()) {
                if (it->second.mNet == svr) {
                    pTable->erase(it++);
                    if (pTable->empty()) {
                        mRouteTable.erase(rtIt);
                        SAFE_DELETE(pTable);
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
		ListNode_t* pNode = etIt->second;
		if (pNode != NULL) {
			ListNodeIt_t it = pNode->begin();
			while (it != pNode->end()) {
				if (it->mNet == svr) {
					pNode->erase(it++);
					if (pNode->empty()) {
                        mErrTable.erase(etIt);
                        SAFE_DELETE(pNode);
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

