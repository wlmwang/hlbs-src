
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "Detect.h"
#include "DetectThread.h"

DetectThread::DetectThread() : mPollFD(kFDUnknown), mNowTm(0), mLocalIp(0), mDetectLoopUsleep(100000),
mDetectMaxNode(10000), mDetectNodeInterval(10), mPingTimeout(0.1), mTcpTimeout(0.8) {
	mPing = new wPing();
	mSocket = new wTcpSocket();
	mDetectMutex = new wMutex();
	mResultMutex = new wMutex();
}

DetectThread::~DetectThread() {
	SAFE_DELETE(mPing);
	SAFE_DELETE(mSocket);
	SAFE_DELETE(mDetectMutex);
	SAFE_DELETE(mResultMutex);
}

const wStatus& DetectThread::PrepareRun() {
	mLocalIp = GetIpByIF("eth1")? GetIpByIF("eth1"): (GetIpByIF("eth0")? GetIpByIF("eth0"): 0);
    return mStatus.Clear();
}

const wStatus& DetectThread::Run() {
	char sIp[32] = {0};
	inet_ntop(AF_INET, &mLocalIp, sIp, sizeof(sIp));

	mNowTm = time(NULL);
    time_t nextReadtm = mNowTm;
    time_t readIntervaltm = 60;

	while (true) {
		mDetectMutex->Lock();

		if (mDetectMapNewadd.empty() && mDetectMapAll.empty()) {
            mDetectMapNewdel.clear();

            mDetectMutex->Unlock();
            // 100ms
            usleep(mDetectLoopUsleep);
            continue;
		}

		MapDetect_t stlNewadd, stlNewdel;
        mDetectMapNewadd.swap(stlNewadd);
        mDetectMapNewdel.swap(stlNewdel);

        mDetectMutex->Unlock();

        if (!stlNewadd.empty() || !stlNewdel.empty()) {
        	//
        }

        // 删除探测节点; 仅需在删除时, 最小粒度加锁，最小粒度不阻塞查询
        if (!stlNewdel.empty()) {
            mResultMutex->Lock();
            for(MapDetectIt_t itDel = stlNewdel.begin(); itDel != stlNewdel.end(); ++itDel) {
                const struct DetectNode_t& node = itDel->first;
                std::map<struct DetectNode_t, struct DetectResult_t>::iterator itFind = mDetectMapAll.find(node);
                if (itFind != mDetectMapAll.end()) {
                    mDetectMapAll.erase(itFind);
                }
            }
            mResultMutex->Unlock();
        }

        // 优先探测上次新增的节点
        for(MapDetectIt_t itAdd = stlNewadd.begin(); itAdd != stlNewadd.end(); ++itAdd) {
            const struct DetectNode_t& node = itAdd->first;
            struct DetectResult_t& res = mDetectMapAll[node];

            // 探测间隔
            if (res.mNextDetectTime > 0 && res.mNextDetectTime >= mNowTm) {
                continue;
            }
            DoDetectNode(node, res);
        }

        // 过期的节点
        std::vector<struct DetectNode_t> vExpireNode;

        // 探测所有节点
        for(MapDetectIt_t it = mDetectMapAll.begin(); it != mDetectMapAll.end(); ++it) {
            const struct DetectNode_t& node = it->first;
            struct DetectResult_t& res = mDetectMapAll[node];

            // 探测过期
            if (node.mExpireTime > 0 && node.mExpireTime <= mNowTm) {
                vExpireNode.push_back(node);
                continue;
            }

            // 探测间隔
            if (res.mNextDetectTime > 0 && res.mNextDetectTime >= mNowTm) {
                continue;
            }
            DoDetectNode(node, res);
		}

        // 删除过期的节点, 仅需在删除时, 最小粒度加锁，最小粒度不阻塞查询
        uint32_t iExpireNodeSize = vExpireNode.size();
        if (iExpireNodeSize > 0) {
            mResultMutex->Lock();
            for(uint32_t i = 0; i < iExpireNodeSize; ++i) {
                struct DetectNode_t& stNode = vExpireNode[i];

                MapDetectIt_t itFind = mDetectMapAll.find(stNode);
                if (itFind != mDetectMapAll.end()) {
                    mDetectMapAll.erase(itFind);
                }
            }
            mResultMutex->Unlock();
        }

        uint32_t iDetectCount = mDetectMapAll.size();
        if (iDetectCount > 0 || iExpireNodeSize > 0) {
            if (iDetectCount >= mDetectMaxNode) {
            	// 清除
                mResultMutex->Lock();
                mDetectMapAll.clear();
                mResultMutex->Unlock();
            }
            if (nextReadtm + readIntervaltm < mNowTm) {
                nextReadtm = mNowTm;
            }
        }
        usleep(mDetectLoopUsleep);
	}
	return mStatus.Clear();
}

const wStatus& DetectThread::DoDetectNode(const struct DetectNode_t& node, struct DetectResult_t& res) {
    struct timeval stOrgtv, stStarttv, stEndtv;
    misc::GetTimeofday(&stStarttv);
    stOrgtv.tv_sec = stStarttv.tv_sec;
    stOrgtv.tv_usec = stStarttv.tv_usec;

    mNowTm = time(NULL);

    if (!(mStatus = mPing->Open()).Ok()) {
    	return mStatus;
    } else if (!(mStatus = mPing->SetTimeout(mPingTimeout)).Ok()) {
    	return mStatus;
    }

    // ping detect start
    mStatus = mPing->Ping(node.mIp.c_str());
    misc::GetTimeofday(&stEndtv);

    int64_t pingElapse = -1, connElapse = -1, allElapse = -1, elapse = -1;
    int32_t ret = 0, detectType = DETECT_UNKNOWN;

    if (!mStatus.Ok()) {
    	// ping detect failed
    	ret = -1;
    } else {
    	// ping detect success
    	detectType = DETECT_PING;

		// ping花费微妙延时
    	pingElapse = static_cast<int64_t>((stEndtv.tv_sec - stStarttv.tv_sec)*1000000 + (stEndtv.tv_usec - stStarttv.tv_usec));
    	
		// TCP detect start
		misc::GetTimeofday(&stStarttv);
    	if ((mStatus = mSocket->Open()).Ok()) {
    		mStatus = mSocket->Connect(node.mIp.c_str(), node.mPort, mTcpTimeout);
    	}

    	if (!mStatus.Ok()) {
    		// TCP detect failed
    		ret = 0;
    	} else {
    		// TCP detect success
    		// 快速关闭TCP socket
    		mSocket->Close();

    		detectType = DETECT_TCP;
    		misc::GetTimeofday(&stEndtv);
    		// connect花费微妙延时
    		connElapse = static_cast<int64_t>((stEndtv.tv_sec - stStarttv.tv_sec)*1000000 + (stEndtv.tv_usec - stStarttv.tv_usec));
    	}
    }

    // detect花费微妙延时
    misc::GetTimeofday(&stEndtv);
    allElapse = static_cast<int64_t>((stEndtv.tv_sec - stOrgtv.tv_sec)*1000000 + (stEndtv.tv_usec - stOrgtv.tv_usec));
    
    if (ret == 0) {
    	elapse = connElapse == -1 ? pingElapse : connElapse;
    } else {
    	//
    }
    
    res.mRc = ret;
    res.mDetectType = detectType;
    res.mElapse = elapse;
    res.mPingElapse = pingElapse;
    res.mConnElapse = connElapse;

    // 设置下次探测时间
    res.HasDetect(mNowTm, mDetectNodeInterval);
    return mStatus.Clear();
}

const wStatus& DetectThread::GetDetectResult(const struct DetectNode_t& node, struct DetectResult_t& res, int32_t* ret) {
    // not found
	mResultMutex->Lock();
    *ret = -1;
    MapDetectIt_t it = mDetectMapAll.find(node);
    if (it != mDetectMapAll.end()) {
    	// has detected
    	*ret = 0;
        res = it->second;
        if (res.mLastDetectTime == 0) {
        	// not complete detect
        	*ret = 1;
        }
    }
    mResultMutex->Unlock();
    return mStatus.Clear();
}

const wStatus& DetectThread::AddDetect(const vector<struct DetectNode_t>& node) {
    mDetectMutex->Lock();
    MapDetectIt_t itdel;
    for (VecCIt_t it = node.begin(); it != node.end(); ++it) {
        const struct DetectNode_t& stNode = *it;

        itdel = mDetectMapNewdel.find(stNode);
        if (itdel != mDetectMapNewdel.end()) {
        	// 删除新加入待删除节点
            mDetectMapNewdel.erase(itdel);
        }

        itdel = mDetectMapNewadd.find(stNode);
        if (itdel != mDetectMapNewadd.end()) {
        	// 更新探测创建时间、过期时间
            struct DetectNode_t &d = const_cast<struct DetectNode_t&>(itdel->first);
            d.Touch(stNode.mCreateTime, stNode.mExpireTime);
        } else {
            // 添加新加入待检测节点
            mDetectMapNewadd.insert(std::make_pair(stNode, DetectResult_t()));
        }
    }
    mDetectMutex->Unlock();
	return mStatus.Clear();
}

const wStatus& DetectThread::DelDetect(const std::vector<struct DetectNode_t>& node) {
    mDetectMutex->Lock();
    MapDetectIt_t itdel;
    for (VecCIt_t it = node.begin(); it != node.end(); ++it) {
        const struct DetectNode_t& stNode = *it;

        itdel = mDetectMapNewadd.find(stNode);
        if (itdel != mDetectMapNewadd.end()) {
        	// 删除新加入待检测节点
            mDetectMapNewadd.erase(itdel);
        }

        // 添加新加入待删除节点
        mDetectMapNewdel.insert(std::make_pair(stNode, DetectResult_t()));
    }
	mDetectMutex->Unlock();
	return mStatus.Clear();
}
