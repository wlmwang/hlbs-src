
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _AGENT_SERVER_H_
#define _AGENT_SERVER_H_

#include <arpa/inet.h>
#include <vector>

#include "wType.h"
#include "wSingleton.h"
#include "wTimer.h"
#include "wTcpServer.h"
#include "wTcpTask.h"
#include "wMTcpClient.h"
#include "wTcpClient.h"
#include "wShm.h"
#include "wMsgQueue.h"
#include "AgentServerTask.h"
#include "AgentConfig.h"
#include "SvrCmd.h"

class AgentServer: public wTcpServer<AgentServer>
{
	public:
		AgentServer();
		virtual ~AgentServer();
		
		void Initialize();
		void InitShareMemory();

		virtual void PrepareRun();
		
		virtual void Run();
		
		void CheckTimer();
		void CheckQueue();
		
		virtual wTcpTask* NewTcpTask(wSocket *pSocket);

		void ConnectRouter();
		int InitSvrReq();
		int ReloadSvrReq();
		
		wMTcpClient<AgentServerTask>* RouterConn()
		{
			return mRouterConn;
		}
		
	private:
		wMTcpClient<AgentServerTask> *mRouterConn;	//连接router
		AgentConfig *mConfig;

		unsigned long long mTicker;
		wTimer mReportTimer;

		wShm *mInShm;	//输入的消息队列的缓冲区位置
		wShm *mOutShm; //输出的消息队列的缓冲区位置
		wMsgQueue* mInMsgQ;	// 输入的消息队列
		wMsgQueue* mOutMsgQ;// 输出的消息队列
};

#endif
