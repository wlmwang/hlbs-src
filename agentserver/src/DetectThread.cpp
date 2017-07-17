
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "wLogger.h"
#include "DetectThread.h"
#include "Detect.h"
#include "Define.h"
#include "Svr.h"

DetectThread::DetectThread(int32_t detectNodeInterval, int32_t detectLoopUsleep, int32_t detectMaxNode): wThread(false),
mPollFD(kFDUnknown), mNowTm(0), mLocalIp(0), mDetectLoopUsleep(detectLoopUsleep), mDetectMaxNode(detectMaxNode), 
mDetectNodeInterval(detectNodeInterval), mPingTimeout(0.1), mTcpTimeout(0.8) {
    char ip[32];
    mLocalIp = misc::GetIpByIF("eth1")? misc::GetIpByIF("eth1"): (misc::GetIpByIF("eth0")? misc::GetIpByIF("eth0"): 0);
    if (!inet_ntop(AF_INET, &mLocalIp, ip, sizeof(ip))) {
        memset(ip, 0, sizeof(ip));
    }
    HNET_NEW(wPing(ip), mPing);
    HNET_NEW(wTcpSocket(), mSocket);
    HNET_NEW(wMutex(), mDetectMutex);
    HNET_NEW(wMutex(), mResultMutex);
}

DetectThread::~DetectThread() {
	HNET_DELETE(mPing);
	HNET_DELETE(mSocket);
	HNET_DELETE(mDetectMutex);
	HNET_DELETE(mResultMutex);
}

int DetectThread::RunThread() {
    time_t nextReadtm = soft::TimeUnix();
    time_t readIntervaltm = 60;

	HNET_DEBUG(kSvrLog, "DetectThread::RunThread detect start, UID(%d), GID(%d), EUID(%d), EGID(%d)", ::getuid(), ::getgid(), ::geteuid(), ::getegid());

	while (true) {
        soft::TimeUpdate();

		mDetectMutex->Lock();
		if (mDetectMapNewadd.empty() && mDetectMapAll.empty()) {
            mDetectMapNewdel.clear();
            mDetectMutex->Unlock();
            usleep(mDetectLoopUsleep);    // 100ms
            continue;
		}

		MapDetect_t stlNewadd, stlNewdel;
        mDetectMapNewadd.swap(stlNewadd);
        mDetectMapNewdel.swap(stlNewdel);
        mDetectMutex->Unlock();

        if (!stlNewadd.empty() || !stlNewdel.empty()) {
        	HNET_DEBUG(kSvrLog, "DetectThread::RunThread newAdd size(%d), newDel size(%d)", stlNewadd.size(), stlNewdel.size());
        }

        mNowTm = soft::TimeUnix();

        // 删除探测节点; 仅需在删除时, 最小粒度加锁，最小粒度不阻塞查询
        if (!stlNewdel.empty()) {
            mResultMutex->Lock();
            for (MapDetectIt_t itDel = stlNewdel.begin(); itDel != stlNewdel.end(); ++itDel) {
                const struct DetectNode_t& node = itDel->first;
                MapDetectIt_t itFind = mDetectMapAll.find(node);
                if (itFind != mDetectMapAll.end()) {
                    mDetectMapAll.erase(itFind);

                    HNET_DEBUG(kSvrLog, "DetectThread::RunThread success to delete detect node %s:%u", node.mIp.c_str(), node.mPort);
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

                	HNET_ERROR(kSvrLog, "DetectThread::RunThread success to delete expire node %s:%u", stNode.mIp.c_str(), stNode.mPort);
                }
            }
            mResultMutex->Unlock();
        }

        uint32_t detectCount = mDetectMapAll.size();
        if (detectCount > 0 || expireNodeSize > 0) {
            if (detectCount >= mDetectMaxNode) {
            	// 清除
            	HNET_ERROR(kSvrLog, "DetectThread::RunThread detect node count(%d) >= detect max node(%d), clean all!", detectCount, mDetectMaxNode);

                mResultMutex->Lock();
            	mDetectMapAll.clear();
            	mResultMutex->Unlock();
            }
            if (nextReadtm + readIntervaltm < mNowTm) {
                nextReadtm = mNowTm;

                HNET_DEBUG(kSvrLog, "DetectThread::RunThread detect node count(%d), expire node count(%d)", detectCount, expireNodeSize);
            }
        }
        usleep(mDetectLoopUsleep);
	}
    
	return 0;
}

int DetectThread::DoDetectNode(const struct DetectNode_t& node, struct DetectResult_t& res) {
    struct timeval stOrgtv, stStarttv, stEndtv;
    
    misc::GetTimeofday(&stEndtv);
    misc::GetTimeofday(&stStarttv);
    stOrgtv.tv_sec = stStarttv.tv_sec;
    stOrgtv.tv_usec = stStarttv.tv_usec;

    mNowTm = soft::TimeUnix();

    int64_t pingElapse = -1, connElapse = -1, allElapse = -1, elapse = -1;
    int32_t ret = 0, rc = 0, detectType = DETECT_UNKNOWN;

    ret = mPing->Open();
    if (ret < 0) {
        return ret;
    }
    mPing->SetTimeout(mPingTimeout);    // 超时时间
    
    ret = mPing->Ping(node.mIp.c_str());    // ping 探测开始
    mPing->Close();

    if (ret < 0) {  // ping 探测失败
    	rc = ret;
    } else {    // ping 探测成功
    	detectType = DETECT_PING;

		// ping 探测花费微妙延时
    	pingElapse = static_cast<int64_t>((stEndtv.tv_sec - stStarttv.tv_sec)*1000000 + (stEndtv.tv_usec - stStarttv.tv_usec));
    	
		// TCP 探测开始
		misc::GetTimeofday(&stStarttv);

        ret = mSocket->Open();
    	if (ret == 0) {
    		ret = mSocket->Connect(node.mIp.c_str(), node.mPort, mTcpTimeout);
            mSocket->Close(); // 快速关闭TCP socket
    	}

    	if (ret == -1) { // TCP 探测失败
    		rc = ret = -1;
    	} else {   // TCP 探测成功
    		rc = ret = 0;
    		detectType = DETECT_TCP;
    		misc::GetTimeofday(&stEndtv);
    		connElapse = static_cast<int64_t>((stEndtv.tv_sec - stStarttv.tv_sec)*1000000 + (stEndtv.tv_usec - stStarttv.tv_usec));   // connect花费微妙延时
    	}
    }

    // detect花费微妙延时
    misc::GetTimeofday(&stEndtv);
    allElapse = static_cast<int64_t>((stEndtv.tv_sec - stOrgtv.tv_sec)*1000000 + (stEndtv.tv_usec - stOrgtv.tv_usec));
    
    if (rc == 0) {
    	elapse = connElapse == -1 ? pingElapse : connElapse;

		HNET_DEBUG(kSvrLog, "DetectThread::DoDetectNode detect Success, HOST(%s),PORT(%d),allElapse(%d),RC(%d),pingElapse(%d),connElapse(%d),elapse(%d),RET(%d)",
				node.mIp.c_str(), node.mPort, allElapse, rc, pingElapse, connElapse, elapse, ret);
    } else {

		HNET_ERROR(kSvrLog, "DetectThread::DoDetectNode detect failed, HOST(%s),PORT(%d),allElapse(%d),RC(%d),pingElapse(%d),connElapse(%d),elapse(%d),RET(%d)",
				node.mIp.c_str(), node.mPort, allElapse, rc, pingElapse, connElapse, elapse, ret);
    }
    
    res.mRc = rc;
    res.mDetectType = detectType;
    res.mElapse = elapse;
    res.mPingElapse = pingElapse;
    res.mConnElapse = connElapse;

    // 设置下次探测时间
    res.HasDetect(mNowTm, mDetectNodeInterval);
    return 0;
}

int DetectThread::GetDetectResult(const struct DetectNode_t& node, struct DetectResult_t& res) {
	int ret = -1;

    mResultMutex->Lock();
    MapDetectIt_t it = mDetectMapAll.find(node);
    if (it != mDetectMapAll.end()) {    // 有探测路由
    	ret = 0;
        res = it->second;
        if (res.mLastDetectTime == 0) { // 刚初始化，还未探测
        	ret = 1;
        }
    }
    mResultMutex->Unlock();
    return ret;
}

int DetectThread::AddDetect(const VecNode_t& node) {
    mDetectMutex->Lock();
    MapDetectIt_t itdel;
    for (VecNodeIt_t it = node.begin(); it != node.end(); ++it) {
        const struct DetectNode_t& stNode = *it;

        itdel = mDetectMapNewdel.find(stNode);
        if (itdel != mDetectMapNewdel.end()) {  // 删除新加入待删除节点
            mDetectMapNewdel.erase(itdel);
        }

        itdel = mDetectMapNewadd.find(stNode);
        if (itdel != mDetectMapNewadd.end()) {  // 更新已加入探测节点的 创建时间、过期时间
            struct DetectNode_t &d = const_cast<struct DetectNode_t&>(itdel->first);
            d.Touch(stNode.mCreateTime, stNode.mExpireTime);
        } else {    // 添加新加入待检测节点
            mDetectMapNewadd.insert(std::make_pair(stNode, DetectResult_t()));
        }
    }
    mDetectMutex->Unlock();
	return 0;
}

int DetectThread::DelDetect(const VecNode_t& node) {
    mDetectMutex->Lock();
    MapDetectIt_t itdel;
    for (VecNodeIt_t it = node.begin(); it != node.end(); ++it) {
        const struct DetectNode_t& stNode = *it;

        itdel = mDetectMapNewadd.find(stNode);
        if (itdel != mDetectMapNewadd.end()) {  // 删除新加入待检测节点
            mDetectMapNewadd.erase(itdel);
        }
        // 添加新加入待删除节点
        mDetectMapNewdel.insert(std::make_pair(stNode, DetectResult_t()));
    }
	mDetectMutex->Unlock();
	return 0;
}
