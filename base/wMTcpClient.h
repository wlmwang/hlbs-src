
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_MTCP_CLIENT_H_
#define _W_MTCP_CLIENT_H_

#include <algorithm>
#include <map>
#include <vector>

#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "wCore.h"
#include "wTimer.h"
#include "wMisc.h"
#include "wLog.h"
#include "wNoncopyable.h"
#include "wTcpClient.h"

template <typename TASK>
class wMTcpClient : public wNoncopyable
{
	public:
		wMTcpClient();
		~wMTcpClient();
		
		void Final();
		void Initialize();
		
		bool GenerateClient(int iType, string sClientName, char *vIPAddress, unsigned short vPort);
		
		bool RemoveTcpClientPool(int iType, wTcpClient<TASK> *pTcpClient = NULL);
		void CleanTcpClientPool();
		int ResetTcpClientCount();
		
		/**
		 * epoll|socket相关
		 */
		int InitEpoll();
		void CleanEpoll();
		int AddToEpoll(wTcpClient<TASK> *pTcpClient);
        int RemoveEpoll(wTcpClient<TASK> *pTcpClient);
		
		void PrepareStart();
		void Start(bool bDaemon = true);
		
		virtual void Run();
		virtual void PrepareRun();
		
		void CheckTimer();
		void Recv();
		void Send();
		
		bool IsRunning() { return CLIENT_RUNNING == mStatus; }
		
		void SetStatus(CLIENT_STATUS eStatus = CLIENT_QUIT) { mStatus = eStatus; }
		CLIENT_STATUS GetStatus() { return mStatus; }

		vector<wTcpClient<TASK>*> TcpClients(int iType);
		wTcpClient<TASK>* OneTcpClient(int iType);
		
	protected:
		wTcpClient<TASK>* CreateClient(int iType, string sClientName, char *vIPAddress, unsigned short vPort);
		bool AddTcpClientPool(int iType, wTcpClient<TASK> *pTcpClient);
		
		CLIENT_STATUS mStatus;	//服务器当前状态

		//epoll
		int mEpollFD;
		int mTimeout;	//epoll_wait timeout
		
		//epoll_event
		struct epoll_event mEpollEvent;
		vector<struct epoll_event> mEpollEventPool; //epoll_event已发生事件池（epoll_wait）
		
		int mTcpClientCount;
		
		//定时记录器
		unsigned long long mLastTicker;	//服务器当前时间
		bool mIsCheckTimer;
        
		std::map<int, vector<wTcpClient<TASK>*> > mTcpClientPool;	//每种类型客户端，可挂载多个连接
};

#include "wMTcpClient.inl"

#endif
