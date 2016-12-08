
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "Detect.h"
#include "DetectThread.h"

DetectThread::DetectThread() : mPollFD(kFDUnknown), mNowTm(0), mLocalIp(0), mDetectLoopUsleep(100000),
mDetectMaxNode(1000), mDetectNodeInterval(10), mPingTimeout(0.1), mTcpTimeout(0.8) {
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

    time_t nextReadtm = time(NULL);
    time_t readIntervaltm = 60;

	while (true) {
		mDetectMutex->Lock();
		if (mDetectMapNewadd.empty() && mDetectMapAll.empty()) {
            mDetectMapNewdel.clear();

            mDetectMutex->Unlock();
            usleep(mDetectLoopUsleep);
            continue;
		}

        std::map<struct DetectNode_t, struct DetectResult_t> stlNewadd;
        std::map<struct DetectNode_t, struct DetectResult_t> stlNewdel;
        mDetectMapNewadd.swap(stlNewadd);
        mDetectMapNewdel.swap(stlNewdel);
        mDetectMutex->Unlock();

        if (!stlNewadd.empty() || !stlNewdel.empty()) {
        	LOG_DEBUG(ELOG_KEY, "[detect] newadd node size=%u, delnode size=%u",stlNewadd.size(), stlNewdel.size());
        }

		mNowTm = time(NULL);

        //删除探测节点; 仅需在删除时, 最小粒度加锁，最小粒度不阻塞查询
        if (!stlNewdel.empty()) {
            mResultMutex->Lock();
            std::map<struct DetectNode_t, struct DetectResult_t>::iterator itDel = stlNewdel.begin();
            for(; itDel != stlNewdel.end(); ++itDel) {
                const struct DetectNode_t &stNode = itDel->first;
                std::map<struct DetectNode_t, struct DetectResult_t>::iterator itFind = mDetectMapAll.find(stNode);
                if (itFind != mDetectMapAll.end()) {
                    mDetectMapAll.erase(itFind);

                    LOG_DEBUG(ELOG_KEY, "[detect] success to delete detect node %s:%u, real life=%d, limit life=%d",
                    	stNode.mIp.c_str(), stNode.mPort, mNowTm - stNode.mCreateTime, stNode.mExpireTime - stNode.mCreateTime);
                }
            }
            mResultMutex->Unlock();
        }

        //优先探测上次新增的节点
        std::map<struct DetectNode_t, struct DetectResult_t>::iterator itAdd = stlNewadd.begin();
        for(; itAdd != stlNewadd.end(); ++itAdd) {
            const struct DetectNode_t &stNode = itAdd->first;
            struct DetectResult_t &stRes = mDetectMapAll[stNode];

            if (stRes.mNextDetectTime > 0 && stRes.mNextDetectTime >= mNowTm) {
                continue;
            }
            DoDetectNode(stNode, stRes);
        }

        vector<struct DetectNode_t> vExpireNode; // 过期的节点

        //探测所有节点
        std::map<struct DetectNode_t, struct DetectResult_t>::iterator it = mDetectMapAll.begin();
        for(; it != mDetectMapAll.end(); ++it) {
            const struct DetectNode_t &stNode = it->first;
            struct DetectResult_t &stRes = mDetectMapAll[stNode];

            if (stNode.mExpireTime > 0 && stNode.mExpireTime <= mNowTm) {
                vExpireNode.push_back(stNode);
                continue;
            }

            if (stRes.mNextDetectTime > 0 && stRes.mNextDetectTime >= mNowTm) {
                continue;
            }
            DoDetectNode(stNode, stRes);
		}

        //删除过期的节点, 仅需在删除时, 最小粒度加锁，最小粒度不阻塞查询
        unsigned int iExpireNodeSize = vExpireNode.size();
        if (iExpireNodeSize > 0) {
            mResultMutex->Lock();
            for(unsigned int i = 0; i < iExpireNodeSize; ++i) {
                struct DetectNode_t &stNode = vExpireNode[i];

                std::map<struct DetectNode_t, struct DetectResult_t>::iterator itFind = mDetectMapAll.find(stNode);
                if (itFind != mDetectMapAll.end()) {
                    mDetectMapAll.erase(itFind);

                    LOG_DEBUG(ELOG_KEY, "[detect] success to delete expire node %s:%u, real life=%d, limit life=%d",
                    	stNode.mIp.c_str(), stNode.mPort, mNowTm - stNode.mCreateTime, stNode.mExpireTime - stNode.mCreateTime);
                }
            }
            mResultMutex->Unlock();
        }

        unsigned int iDetectCount = mDetectMapAll.size();
        if (iDetectCount > 0 || iExpireNodeSize > 0) {
            if (iDetectCount >= mDetectMaxNode) {
                LOG_DEBUG(ELOG_KEY, "[detect] detect node count=%u >= detect max node=%u!!! clean !!",iDetectCount, mDetectMaxNode);
                mResultMutex->Lock();
                mDetectMapAll.clear();
                mResultMutex->Unlock();
            }
            if (nextReadtm + readIntervaltm < mNowTm) {
                nextReadtm = mNowTm;
                LOG_DEBUG(ELOG_KEY, "[detect] [stat] detect node count=%u, expire node count=%u",iDetectCount, iExpireNodeSize);
            }
        }
        usleep(mDetectLoopUsleep);
	}

	return 0;
}

int DetectThread::DoDetectNode(const struct DetectNode_t& stNode, struct DetectResult_t& stRes) {
	int iRet = -1, iRc = 0, iPingElapse, iConnElapse, iUdpElapse, iAllElapse, iElapse, iDetectType = DETECT_UNKNOWN;
	struct timeval stStarttv, stEndtv, stOrgtv;
	
	iPingElapse = iConnElapse = iUdpElapse = iAllElapse = iElapse = -1;
    time_t iNowTm = time(NULL);
    gettimeofday(&stStarttv, 0);
	
    stOrgtv.tv_sec = stStarttv.tv_sec;
    stOrgtv.tv_usec = stStarttv.tv_usec;
    if ((iRet = mPing->Open()) > 0 && mPing->SetTimeout(mPingTimeout) >= 0) {
    	iRet = mPing->Ping(stNode.mIp.c_str());
    }

    gettimeofday(&stEndtv, 0);
    
    if (iRet < 0) {
    	//ping failed
    	iRc = iRet;
    } else {
    	//tcp connect
		iDetectType = DETECT_PING;
		iPingElapse = (int)((stEndtv.tv_sec - stStarttv.tv_sec)*1000000 + (stEndtv.tv_usec - stStarttv.tv_usec));
    	
		gettimeofday(&stStarttv, 0);
    	if ((iRet = mSocket->Open()) > 0) {
    		iRet = mSocket->Connect(stNode.mIp.c_str(), stNode.mPort, mTcpTimeout);
    	}

    	if (iRet < 0) {
    		iRc = 0;
    	} else {
    		//快速关闭socket
		    struct linger stLing = {1,0};
		    setsockopt(mSocket->FD(), SOL_SOCKET, SO_LINGER, (const char *) &stLing, sizeof(stLing));
    		mSocket->Close();

    		iDetectType = DETECT_TCP;
            gettimeofday(&stEndtv, 0);
            iConnElapse = (int)((stEndtv.tv_sec - stStarttv.tv_sec)*1000000 + (stEndtv.tv_usec - stStarttv.tv_usec));
    	}
    }

    iAllElapse = (int)((stEndtv.tv_sec - stOrgtv.tv_sec)*1000000 + (stEndtv.tv_usec - stOrgtv.tv_usec));
    
    if (iRc == 0) {
    	iElapse = ((iConnElapse == -1) ? iPingElapse : iConnElapse);

    	LOG_DEBUG(ELOG_KEY, "[detect] succ detect %d us,ip %s:%u rc %d, ping %d,connect %d; elapse=%d,ret=%d",
    		iAllElapse, stNode.mIp.c_str(), stNode.mPort, iRc, iPingElapse, iConnElapse, iElapse, iRet);
    } else {
    	LOG_DEBUG(ELOG_KEY, "[detect] fail detect %d us,ip %s:%u rc %d, ping %d,connect %d; elapse=%d,ret=%d",
    		iAllElapse, stNode.mIp.c_str(), stNode.mPort, iRc, iPingElapse, iConnElapse, iElapse, iRet);
    }
    
    stRes.mRc = iRc;
    stRes.mDetectType = iDetectType;
    stRes.mElapse = iElapse;
    stRes.mPingElapse = iPingElapse;
    stRes.mConnElapse = iConnElapse;
    //stRes.mUdpElapse = iUdpElapse;
    stRes.HasDetect(iNowTm, mDetectNodeInterval); // 设置下次探测时间
    return 0;
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
