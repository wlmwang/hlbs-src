
/**
 * Copyright (C) Anny.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _DETECT_THREAD_H_
#define _DETECT_THREAD_H_

#include <map>
#include <list>
#include <vector>
#include "wCore.h"
#include "wStatus.h"
#include "wNoncopyable.h"
#include "wMutex.h"
#include "wMisc.h"
#include "wThread.h"
#include "wPing.h"
#include "wTcpSocket.h"
#include "SvrCmd.h"

class DetectThread : public wThread {
	typedef std::map<struct DetectNode_t, struct DetectResult_t> MapDetect_t;
	typedef std::map<struct DetectNode_t, struct DetectResult_t>::iterator MapDetectIt_t;
	typedef std::vector<struct DetectNode_t> VecNode_t;
	typedef std::vector<struct DetectNode_t>::const_iterator VecNodeIt_t;
public:
	DetectThread();
	virtual ~DetectThread();

	virtual const wStatus& PrepareThread();
    virtual const wStatus& RunThread();

    const wStatus& DelDetect(const VecNode_t& node);
	const wStatus& AddDetect(const VecNode_t& node);
	const wStatus& GetDetectResult(const struct DetectNode_t& node, struct DetectResult_t& res, int32_t* ret);
	const wStatus& DoDetectNode(const struct DetectNode_t& stNode, struct DetectResult_t& stRes);

protected:
	wPing *mPing;
	wSocket *mSocket;
	int32_t mPollFD;

	time_t mNowTm;
	uint32_t mLocalIp;
	uint32_t mDetectLoopUsleep;
	uint32_t mDetectMaxNode;
	uint32_t mDetectNodeInterval;

	float mPingTimeout;
	float mTcpTimeout;

	wMutex *mDetectMutex;
	wMutex *mResultMutex;
	MapDetect_t mDetectMapAll;		// 检测队列
	MapDetect_t mDetectMapNewadd;	// 新加入待检测节点，优先探测
	MapDetect_t mDetectMapNewdel;	// 新加入待删除节点，优先探测

	wStatus mStatus;
};

#endif
