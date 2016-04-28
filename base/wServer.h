
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _W_SERVER_H_
#define _W_SERVER_H_

#include <algorithm>
#include <vector>
#include <sys/epoll.h>

#include "wCore.h"
#include "wAssert.h"
#include "wMisc.h"
#include "wLog.h"
#include "wSingleton.h"
#include "wTimer.h"
#include "wTask.h"
#include "wWorker.h"
#include "wMaster.h"
#include "wSocket.h"
#include "wHttp.h"
#include "wTcpTask.h"
#include "wHttpTask.h"
#include "wChannelTask.h"

template <typename T>
class wServer: public wSingleton<T>
{
	public:
		wServer(string ServerName);
		void Initialize();
		virtual ~wServer();

		void Final();
		
		/**
		 * 事件读写主调函数
		 */
		void Recv();
		int Send(wTask *pTask, const char *pCmd, int iLen);
		void Broadcast(const char *pCmd, int iLen);
		
		/**
		 * single、worker进程中，准备|启动服务
		 */
		void PrepareStart(string sIpAddr ,unsigned int nPort);
		void Start(bool bDaemon = true);
		
		/**
		 * master-worker用户多进程架构，准备|启动服务。（防止bind失败）
		 *
		 * PrepareMaster 需在master进程中调用
		 * WorkerStart在worker进程提供服务
		 */
		void PrepareMaster(string sIpAddr ,unsigned int nPort);	
		void WorkerStart(wWorker *pWorker = NULL, bool bDaemon = true);
		int AcceptMutexLock();
		int AcceptMutexUnlock();
		virtual void HandleSignal();
		void WorkerExit();
		
		/**
		 * epoll event
		 */
		int InitEpoll();
		void CleanEpoll();
		/**
		 * 添加到epoll监听事件中
		 * @param  pTask [wTask*]
		 * @return       [是否出错]
		 */
		int AddToEpoll(wTask* pTask, int iEvents = EPOLLIN, int iOp = EPOLL_CTL_ADD);
        int RemoveEpoll(wTask* pTask);
		
		/**
		 *  Listen Socket
		 */
		int InitListen(string sIpAddr ,unsigned int nPort);
		
		/**
		 *  accept接受连接
		 */
		int AcceptConn();
		int AddToTaskPool(wTask *pTask);
		void CleanTaskPool();
	    std::vector<wTask*>::iterator RemoveTaskPool(wTask *pTask);
		int PoolNum() { return mTaskPool.size();}

		/**
		 * 新建客户端
		 */
		virtual wTask* NewTcpTask(wIO *pIO);	//io = wSocket
		virtual wTask* NewHttpTask(wIO *pIO);	//io = wHttp
		virtual wTask* NewChannelTask(wIO *pIO);//io = wChannel
		
		/**
		 * 服务主循环逻辑，继承可以定制服务
		 */
		virtual void PrepareRun();
		virtual void Run();
		
		string &ServerName() { return mServerName; }
		bool IsRunning() { return mStatus == SERVER_RUNNING; }
		SERVER_STATUS &Status() { return mStatus; }
		
		void CheckTimer();
		virtual void CheckTimeout();
		
	protected:
		string mServerName;
		wSocket *mListenSock;	//Listen Socket(主服务socket对象)
		
		SERVER_STATUS mStatus;	//服务器当前状态
		unsigned long long mLastTicker;	//服务器当前时间
		wTimer mCheckTimer;
		bool mIsCheckTimer;		//心跳开关，默认关闭。强烈建议移动互联网环境下打开，而非依赖keepalive机制保活
		
		//epoll
		int mEpollFD;
		int mTimeout;	//epoll_wait timeout
		
		//epoll_event
		struct epoll_event mEpollEvent;
		vector<struct epoll_event> mEpollEventPool; //epoll_event已发生事件池（epoll_wait）
		int mTaskCount;	//mTcpTaskPool.size();
		
		//task|pool
		wTask *mTask;		//temp task
		vector<wTask*> mTaskPool;
		
		//worker
		wWorker *mWorker;	//对应worker对象，worker与server一对一（single进程模式无worker进程）
		wChannel *mChannelSock;	//Unix Socket(进程间通信)

		int mErr;
		int mExiting;
};

#include "wServer.inl"

#endif
