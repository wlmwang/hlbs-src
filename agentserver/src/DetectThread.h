
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _DETECT_THREAD_H_
#define _DETECT_THREAD_H_

#include "wCore.h"
#include "wMisc.h"
#include "wThread.h"
#include "wPing.h"
#include "SvrQos.h"

class SvrQos;
class DetectThread : public wThread
{
	friend class SvrQos;
	public:
		DetectThread();
		virtual ~DetectThread();

		virtual int PrepareRun();
		virtual int Run();
		virtual bool IsBlocked();
		
		//int DelDetect(const vector<detect_node> &nodevec);
		//int AddDetect(const vector<detect_node> &nodevec);
		//int GetDetectResult(detect_node &node, detect_result& result);

	protected:
		unsigned int mLocalIp;
		wPing *mPing;
		wMutex *mDeteMutex;
		//map<detect_node, detect_result> mDetectMapAll;
		//map<detect_node, detect_result> mDetectMapNewadd; //新加入的,优先探测
		//map<detect_node, detect_result> mDetectMapNewdel; //新删除的,优先探测
};

#endif