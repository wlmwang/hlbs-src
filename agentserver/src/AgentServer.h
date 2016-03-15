
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _AGENT_SERVER_H_
#define _AGENT_SERVER_H_

#include <arpa/inet.h>
#include <vector>

#include "wCore.h"
#include "wSingleton.h"
#include "wTimer.h"
#include "wTcpServer.h"
#include "wTcpTask.h"
#include "wMTcpClient.h"
#include "wTcpClient.h"
#include "wShm.h"
#include "wMsgQueue.h"
#include "AgentServerTask.h"
#include "AgentClientTask.h"
#include "AgentConfig.h"
#include "Common.h"
#include "SvrCmd.h"

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
		int InitSvrReq();
		int ReloadSvrReq();
		wMTcpClient<AgentClientTask>* RouterConn() { return mRouterConn; }

		virtual wTask* NewTcpTask(wIO *pIO);
		
	private:
		unsigned long long mTicker;
		wTimer mReportTimer;

		wMTcpClient<AgentClientTask> *mRouterConn;	//连接router
		
		AgentConfig *mConfig;

		wShm *mInShm;	//输入的消息队列的缓冲区位置
		wShm *mOutShm; //输出的消息队列的缓冲区位置
		wMsgQueue* mInMsgQ;	// 输入的消息队列
		wMsgQueue* mOutMsgQ;// 输出的消息队列
};

#endif
