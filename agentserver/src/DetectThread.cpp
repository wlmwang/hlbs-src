
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "Detect.h"
#include "DetectThread.h"
#include "wLogger.h"
#include "Svr.h"

DetectThread::DetectThread(int32_t detectNodeInterval, int32_t detectLoopUsleep, int32_t detectMaxNode) : mPollFD(kFDUnknown), mNowTm(0), mLocalIp(0), mDetectLoopUsleep(detectLoopUsleep),
mDetectMaxNode(detectMaxNode), mDetectNodeInterval(detectNodeInterval), mPingTimeout(0.1), mTcpTimeout(0.8) {
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

const wStatus& DetectThread::PrepareThread() {
	mLocalIp = misc::GetIpByIF("eth1")? misc::GetIpByIF("eth1"): (misc::GetIpByIF("eth0")? misc::GetIpByIF("eth0"): 0);
    return mStatus;
}

const wStatus& DetectThread::RunThread() {
	// 本机字符串IP地址
	char ip[32] = {0};
	::inet_ntop(AF_INET, &mLocalIp, ip, sizeof(ip));

    time_t nextReadtm = time(NULL);
    time_t readIntervaltm = 60;

	LOG_DEBUG(kSvrLog, "DetectThread::RunThread detect start, UID(%d),GID(%d),EUID(%d),EGID(%d)", ::getuid(), ::getgid(), ::geteuid(), ::getegid());

	while (true) {
		mDetectMutex->Lock();
		if (mDetectMapNewadd.empty() && mDetectMapAll.empty()) {
            mDetectMapNewdel.clear();
            mDetectMutex->Unlock();
            // 100ms
            ::usleep(mDetectLoopUsleep);
            continue;
		}

		MapDetect_t stlNewadd, stlNewdel;
        mDetectMapNewadd.swap(stlNewadd);
        mDetectMapNewdel.swap(stlNewdel);
        mDetectMutex->Unlock();

        if (!stlNewadd.empty() || !stlNewdel.empty()) {
        	LOG_DEBUG(kSvrLog, "DetectThread::RunThread newAdd size(%d), newDel size(%d)", stlNewadd.size(), stlNewdel.size());
        }
        mNowTm = time(NULL);

        // 删除探测节点; 仅需在删除时, 最小粒度加锁，最小粒度不阻塞查询
        if (!stlNewdel.empty()) {
            mResultMutex->Lock();
            for (MapDetectIt_t itDel = stlNewdel.begin(); itDel != stlNewdel.end(); ++itDel) {
                const struct DetectNode_t& node = itDel->first;
                MapDetectIt_t itFind = mDetectMapAll.find(node);
                if (itFind != mDetectMapAll.end()) {
                    mDetectMapAll.erase(itFind);

                    LOG_DEBUG(kSvrLog, "DetectThread::RunThread success to delete detect node %s:%u", node.mIp.c_str(), node.mPort);
                }
            }
            mResultMutex->Unlock();
        }

        // 优先探测上次新增的节点
        for (MapDetectIt_t itAdd = stlNewadd.begin(); itAdd != stlNewadd.end(); ++itAdd) {
            const struct DetectNode_t& node = itAdd->first;
            struct DetectResult_t& res = mDetectMapAll[node];
            // 探测间隔
            if (res.mNextDetectTime > 0 && res.mNextDetectTime >= mNowTm) {
                continue;
            }
            DoDetectNode(node, res);
        }

        // 过期的节点
        VecNode_t expireNode;

        // 探测所有节点
        for (MapDetectIt_t it = mDetectMapAll.begin(); it != mDetectMapAll.end(); ++it) {
            const struct DetectNode_t& node = it->first;
            struct DetectResult_t& res = mDetectMapAll[node];
            // 探测过期
            if (node.mExpireTime > 0 && node.mExpireTime <= mNowTm) {
            	expireNode.push_back(node);
                continue;
            }

            // 探测间隔
            if (res.mNextDetectTime > 0 && res.mNextDetectTime >= mNowTm) {
                continue;
            }
            DoDetectNode(node, res);
		}

        // 删除过期的节点, 仅需在删除时, 最小粒度加锁，最小粒度不阻塞查询
        uint32_t expireNodeSize = expireNode.size();
        if (expireNodeSize > 0) {
            mResultMutex->Lock();
            for(uint32_t i = 0; i < expireNodeSize; ++i) {
                struct DetectNode_t& stNode = expireNode[i];
                MapDetectIt_t itFind = mDetectMapAll.find(stNode);
                if (itFind != mDetectMapAll.end()) {
                    mDetectMapAll.erase(itFind);

                	LOG_ERROR(kSvrLog, "DetectThread::RunThread success to delete expire node %s:%u", stNode.mIp.c_str(), stNode.mPort);
                }
            }
            mResultMutex->Unlock();
        }

        uint32_t detectCount = mDetectMapAll.size();
        if (detectCount > 0 || expireNodeSize > 0) {
            if (detectCount >= mDetectMaxNode) {
            	// 清除
            	LOG_ERROR(kSvrLog, "DetectThread::RunThread detect node count(%d) >= detect max node(%d), clean all!", detectCount, mDetectMaxNode);

                mResultMutex->Lock();
            	mDetectMapAll.clear();
            	mResultMutex->Unlock();
            }
            if (nextReadtm + readIntervaltm < mNowTm) {
                nextReadtm = mNowTm;

                LOG_DEBUG(kSvrLog, "DetectThread::RunThread detect node count(%d), expire node count(%d)", detectCount, expireNodeSize);
            }
        }
        ::usleep(mDetectLoopUsleep);
	}
	return mStatus;
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

    // ping 探测开始
    mStatus = mPing->Ping(node.mIp.c_str());
    misc::GetTimeofday(&stEndtv);

    int64_t pingElapse = -1, connElapse = -1, allElapse = -1, elapse = -1;
    int32_t ret = 0, rc = 0, detectType = DETECT_UNKNOWN;

    if (!mStatus.Ok()) {
    	// ping 探测失败
    	rc = ret = -1;
    } else {
    	// ping 探测成功
    	detectType = DETECT_PING;

		// ping 探测花费微妙延时
    	pingElapse = static_cast<int64_t>((stEndtv.tv_sec - stStarttv.tv_sec)*1000000 + (stEndtv.tv_usec - stStarttv.tv_usec));
    	
		// TCP 探测开始
		misc::GetTimeofday(&stStarttv);
    	if ((mStatus = mSocket->Open()).Ok()) {
    		int64_t ret;
    		mStatus = mSocket->Connect(&ret, node.mIp.c_str(), node.mPort, mTcpTimeout);
    	}

    	if (!mStatus.Ok()) {
    		// TCP 探测失败
    		rc = ret = -1;
    	} else {
    		// TCP 探测成功
    		rc = ret = 0;
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
    
    if (rc == 0) {
    	elapse = connElapse == -1 ? pingElapse : connElapse;
		LOG_DEBUG(kSvrLog, "DetectThread::DoDetectNode detect Success, HOST(%s),PORT(%d),allElapse(%d),RC(%d),pingElapse(%d),connElapse(%d),elapse(%d),RET(%d)",
				node.mIp.c_str(), node.mPort, allElapse, rc, pingElapse, connElapse, elapse, ret);
    } else {
		LOG_ERROR(kSvrLog, "DetectThread::DoDetectNode detect failed, HOST(%s),PORT(%d),allElapse(%d),RC(%d),pingElapse(%d),connElapse(%d),elapse(%d),RET(%d)",
				node.mIp.c_str(), node.mPort, allElapse, rc, pingElapse, connElapse, elapse, ret);
    }
    
    res.mRc = rc;
    res.mDetectType = detectType;
    res.mElapse = elapse;
    res.mPingElapse = pingElapse;
    res.mConnElapse = connElapse;

    // 设置下次探测时间
    res.HasDetect(mNowTm, mDetectNodeInterval);
    return mStatus.Clear();
}

const wStatus& DetectThread::GetDetectResult(const struct DetectNode_t& node, struct DetectResult_t& res, int32_t* ret) {
	mResultMutex->Lock();
    *ret = -1;	// 无探测节点
    MapDetectIt_t it = mDetectMapAll.find(node);
    if (it != mDetectMapAll.end()) {
    	// 有探测路由
    	*ret = 0;
        res = it->second;
        if (res.mLastDetectTime == 0) {
        	// 刚初始化，还未探测
        	*ret = 1;
        }
    }
    mResultMutex->Unlock();
    return mStatus.Clear();
}

const wStatus& DetectThread::AddDetect(const VecNode_t& node) {
    mDetectMutex->Lock();
    MapDetectIt_t itdel;
    for (VecNodeIt_t it = node.begin(); it != node.end(); ++it) {
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

const wStatus& DetectThread::DelDetect(const VecNode_t& node) {
    mDetectMutex->Lock();
    MapDetectIt_t itdel;
    for (VecNodeIt_t it = node.begin(); it != node.end(); ++it) {
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
