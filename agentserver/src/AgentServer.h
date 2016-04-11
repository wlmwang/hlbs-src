
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _AGENT_SERVER_H_
#define _AGENT_SERVER_H_

#include <arpa/inet.h>
#include <vector>

#include "wCore.h"
#include "wAssert.h"
#include "wSingleton.h"
#include "wTimer.h"
#include "wLog.h"
#include "wMisc.h"
#include "wTcpServer.h"
#include "wMTcpClient.h"
#include "wShm.h"
#include "wMsgQueue.h"
#include "Common.h"
#include "SvrCmd.h"
#include "AgentClientTask.h"
#include "AgentServerTask.h"
#include "DetectThread.h"

class AgentServer: public wTcpServer<AgentServer>
{
	public:
		AgentServer();
		virtual ~AgentServer();
		
		void Initialize();
		void InitShm();

		virtual void PrepareRun();
		virtual void Run();
		
		void CheckTimer();
		void CheckQueue();

		void ConnectRouter();

		//连接router成功后，发送初始化svr请求
		int InitSvrReq();

		//可运行时发送重载svr请求
		int ReloadSvrReq();
		wMTcpClient<AgentClientTask>* RouterConn() { return mRouterConn; }

		virtual wTask* NewTcpTask(wIO *pIO);
		
	private:
		unsigned long long mTicker;
		
		AgentConfig *mConfig;
		DetectThread *mDetectThread;
		
		wShm *mInShm;	//输入的消息队列的缓冲区位置
		wShm *mOutShm; //输出的消息队列的缓冲区位置
		wMsgQueue* mInMsgQ;	// 输入的消息队列
		wMsgQueue* mOutMsgQ;// 输出的消息队列
		
		wMTcpClient<AgentClientTask> *mRouterConn;	//连接router
};

#endif
