
/**
 * Copyright (C) Anny.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _DETECT_THREAD_H_
#define _DETECT_THREAD_H_

#include <map>
#include <vector>

#include "wCore.h"
#include "wLog.h"
#include "wMisc.h"
#include "wThread.h"
#include "wSingleton.h"
#include "wPing.h"
#include "wSocket.h"
#include "Detect.h"

class DetectThread : public wThread, public wSingleton<DetectThread>
{
	public:
		DetectThread();
		virtual ~DetectThread();

		virtual int PrepareRun();
		virtual int Run();
		
		int DelDetect(const vector<struct DetectNode_t> &vNode);
		int AddDetect(const vector<struct DetectNode_t> &vNode);
		int GetDetectResult(struct DetectNode_t &stNode, struct DetectResult_t& stRes);
		int DoDetectNode(const struct DetectNode_t& stNode, struct DetectResult_t& stRes);

	protected:
		wPing *mPing;
		wSocket *mSocket;
		int mPollFD;

		unsigned int mLocalIp;
		time_t mNowTm;
		unsigned int mDetectLoopUsleep;
		unsigned int mDetectMaxNode;
		unsigned int mDetectNodeInterval;

		float mPingTimeout;
		float mTcpTimeout;

		wMutex *mDetectMutex;
		wMutex *mResultMutex;
		map<struct DetectNode_t, struct DetectResult_t> mDetectMapAll;		//检测队列
		map<struct DetectNode_t, struct DetectResult_t> mDetectMapNewadd;	//新加入的,优先探测
		map<struct DetectNode_t, struct DetectResult_t> mDetectMapNewdel;	//新删除的,优先探测
};

#endif