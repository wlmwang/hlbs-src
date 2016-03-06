
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_TCP_SERVER_H_
#define _W_TCP_SERVER_H_

#include <algorithm>
#include <vector>
#include <sys/epoll.h>

#include "wType.h"
#include "wAssert.h"
#include "wMisc.h"
#include "wLog.h"
#include "wSingleton.h"
#include "wTimer.h"
#include "wIO.h"
#include "wSocket.h"
#include "wTask.h"
#include "wWorker.h"
#include "wMaster.h"

enum SERVER_STATUS
{
	SERVER_INIT = -1,	//服务器的初始化状态
	SERVER_QUIT,	 	//服务器进入关闭状态
	SERVER_RUNNING	 	//正常运行状态模式
};

template <typename T>
class wTcpServer: public wSingleton<T>
{
	public:
		wTcpServer(string ServerName);
		void Initialize();
		virtual ~wTcpServer();

		void Final();
		
		void Recv();
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
		int AddToEpoll(wTask* pTask);
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
		
		/**
		 * 新建客户端
		 */
		virtual wTask* NewTcpTask(wIO *pIO);
		virtual wTask* NewChannelTask(wIO *pIO);
		
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
		bool mIsCheckTimer;
		
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
};

/** 模板函数实现 */
template <typename T>
wTcpServer<T>::wTcpServer(string ServerName)
{
	Initialize();
	
	mStatus = SERVER_INIT;
	mServerName = ServerName;
}

template <typename T>
wTcpServer<T>::~wTcpServer()
{
	Final();
}

template <typename T>
void wTcpServer<T>::Initialize()
{
	mLastTicker = GetTickCount();
	mCheckTimer = wTimer(KEEPALIVE_TIME);
	mIsCheckTimer = true;
	mServerName = "";
	
	mTimeout = 10;
	mEpollFD = FD_UNKNOWN;
	memset((void *)&mEpollEvent, 0, sizeof(mEpollEvent));
	mTaskCount = 0;
	mEpollEventPool.reserve(512);	//容量
	mTask = NULL;
	mListenSock = NULL;
	
	mChannelSock = NULL;
	mWorker = NULL;
}

template <typename T>
void wTcpServer<T>::Final()
{
	CleanEpoll();
	CleanTaskPool();
	SAFE_DELETE(mListenSock);
}

template <typename T>
void wTcpServer<T>::CleanEpoll()
{
	if(mEpollFD != -1)
	{
		close(mEpollFD);
	}
	mEpollFD = -1;
	memset((void *)&mEpollEvent, 0, sizeof(mEpollEvent));
	mEpollEventPool.clear();
}

template <typename T>
void wTcpServer<T>::CleanTaskPool()
{
	if(mTaskPool.size() > 0)
	{
		vector<wTask*>::iterator iter;
		for(iter = mTaskPool.begin(); iter != mTaskPool.end(); iter++)
		{
			SAFE_DELETE(*iter);
		}
	}
	mTaskPool.clear();
	mTaskCount = 0;
}

template <typename T>
void wTcpServer<T>::Broadcast(const char *pCmd, int iLen)
{
	if(mTaskPool.size() > 0)
	{
		vector<wTask*>::iterator iter;
		for(iter = mTaskPool.begin(); iter != mTaskPool.end(); iter++)
		{
			if ((*iter)->IsRunning() && ((*iter)->IO()->IOFlag() == FLAG_SEND || (*iter)->IO()->IOFlag() == FLAG_RVSD))
			{
				(*iter)->SyncSend(pCmd, iLen);	//同步发送
			}
		}
	}
}

template <typename T>
void wTcpServer<T>::PrepareMaster(string sIpAddr ,unsigned int nPort)
{
	LOG_INFO(ELOG_KEY, "[startup] Master Prepare start succeed");

	//初始化Listen Socket
	if(InitListen(sIpAddr ,nPort) < 0)
	{
		LOG_ERROR(ELOG_KEY, "[startup] InitListen failed: %s", strerror(errno));
		exit(1);
	}

	//运行前工作
	PrepareRun();
}

template <typename T>
void wTcpServer<T>::WorkerStart(wWorker *pWorker, bool bDaemon)
{
	mStatus = SERVER_RUNNING;
	LOG_DEBUG(ELOG_KEY, "[startup] Master start succeed");
	
	//初始化epoll
	if(InitEpoll() < 0)
	{
		LOG_ERROR(ELOG_KEY, "[startup] InitEpoll failed: %s", strerror(errno));
		exit(1);
	}
	
	//Listen Socket 添加到epoll中
	if (mListenSock != NULL)
	{
		mTask = NewTcpTask(mListenSock);
		if(NULL != mTask)
		{
			mTask->Status() = TASK_RUNNING;
			if (AddToEpoll(mTask) >= 0)
			{
				AddToTaskPool(mTask);
			}
		}
	}
	
	//Unix Socket 添加到epoll中（worker自身channel[1]被监听）
	if(pWorker != NULL)
	{
		mWorker = pWorker;
		
		if(mWorker->mWorkerPool != NULL)
		{
			mChannelSock = mWorker->mWorkerPool[mWorker->mSlot]->mCh;	//当前worker进程表项
			if(mChannelSock != NULL)
			{
				//new unix task
				mChannelSock->IOFlag() = FLAG_RECV;
				mChannelSock->SockStatus() = STATUS_LISTEN;
				
				mTask = NewChannelTask(mChannelSock);
				if(NULL != mTask)
				{
					mTask->Status() = TASK_RUNNING;
					if (AddToEpoll(mTask) >= 0)
					{
						AddToTaskPool(mTask);
					}
				}
			}
			else
			{
				LOG_ERROR(ELOG_KEY, "[startup] worker pool slot(%d) illegal", mWorker->mSlot);
				eixt(1);
			}
		}
	}
	
	//进入服务主循环
	do {
		Recv();
		Run();
		if(mIsCheckTimer) CheckTimer();
	} while(IsRunning() && bDaemon);
}

template <typename T>
void wTcpServer<T>::PrepareStart(string sIpAddr ,unsigned int nPort)
{
	LOG_DEBUG(ELOG_KEY, "[startup] Server Prepare start succeed");
	
	//初始化epoll
	if(InitEpoll() < 0)
	{
		LOG_ERROR(ELOG_KEY, "[startup] InitEpoll failed: %s", strerror(errno));
		exit(1);
	}
	
	//初始化Listen Socket
	if(InitListen(sIpAddr ,nPort) < 0)
	{
		LOG_ERROR(ELOG_KEY, "[startup] InitListen failed: %s", strerror(errno));
		exit(1);
	}
	else if(mListenSock != NULL)
	{
		//Listen Socket 添加到epoll中
		mTask = NewTcpTask(mListenSock);
		if(NULL != mTask)
		{
			mTask->Status() = TASK_RUNNING;
			if (AddToEpoll(mTask) >= 0)
			{
				AddToTaskPool(mTask);
			}
		}
	}
	
	//运行前工作
	PrepareRun();
}

template <typename T>
void wTcpServer<T>::Start(bool bDaemon)
{
	mStatus = SERVER_RUNNING;
	LOG_DEBUG(ELOG_KEY, "[startup] Server start succeed");
	
	//进入服务主循环
	do {
		Recv();
		Run();
		if(mIsCheckTimer) CheckTimer();
	} while(IsRunning() && bDaemon);
}

template <typename T>
void wTcpServer<T>::PrepareRun()
{
	//accept前准备工作
}

/**
 * epoll初始化
 */
template <typename T>
int wTcpServer<T>::InitEpoll()
{
	mEpollFD = epoll_create(512); //512
	if(mEpollFD < 0)
	{
		LOG_ERROR(ELOG_KEY, "[startup] epoll_create failed: %s", strerror(errno));
		return -1;
	}
	return mEpollFD;
}

template <typename T>
int wTcpServer<T>::InitListen(string sIpAddr ,unsigned int nPort)
{
	mListenSock = new wSocket();
	int iFD = mListenSock->Open();
	if (iFD == -1)
	{
		return -1;
	}
	mListenSock->IPAddr() = sIpAddr;
	mListenSock->Port() = nPort;
	mListenSock->SockType() = SOCK_LISTEN;
	mListenSock->IOFlag() = FLAG_RECV;
	
	//bind socket
	struct sockaddr_in stSocketAddr;
	stSocketAddr.sin_family = AF_INET;
	stSocketAddr.sin_port = htons((short)nPort);
	stSocketAddr.sin_addr.s_addr = inet_addr(sIpAddr.c_str());
	int iRet = bind(iFD, (struct sockaddr *)&stSocketAddr, sizeof(stSocketAddr));
	if(iRet < 0)
	{
		LOG_ERROR(ELOG_KEY, "[startup] Socket bind failed：%s", strerror(errno));
		SAFE_DELETE(mListenSock);
		return -1;
	}
	//setsockopt socket : 设置发送缓冲大小4M
	int iOptLen = sizeof(socklen_t);
	int iOptVal = 0x400000;
	if(setsockopt(iFD, SOL_SOCKET, SO_SNDBUF, (const void *)&iOptVal, iOptLen) < -1)
	{
		LOG_ERROR(ELOG_KEY, "[startup] Set send buffer size failed");
		SAFE_DELETE(mListenSock);
		return -1;
	}
	
	//listen socket
	iRet = listen(iFD, LISTEN_BACKLOG);
	if(iRet < 0)
	{
		LOG_ERROR(ELOG_KEY, "[startup] listen failed: %s", strerror(errno));
		SAFE_DELETE(mListenSock);
		return -1;
	}
	
	//nonblock
	if(mListenSock->SetNonBlock() < 0) 
	{
		LOG_ERROR(ELOG_KEY, "[startup] Set non block failed: %d, close it", iFD);
		SAFE_DELETE(mListenSock);
		return -1;
	}
	
	mListenSock->SockStatus() = STATUS_LISTEN;	
	return iFD;
}

template <typename T>
wTask* wTcpServer<T>::NewTcpTask(wIO *pIO)
{
	wTask *pTask = new wTcpTask(pIO);
	return pTask;
}

template <typename T>
wTask* wTcpServer<T>::NewChannelTask(wIO *pIO)
{
	wTask *pTask = new wChannelTask(pIO);
	return pTask;
}

template <typename T>
int wTcpServer<T>::AddToEpoll(wTask* pTask, int iEvent = EPOLLIN|EPOLLERR|EPOLLHUP)
{
	mEpollEvent.events = iEvent;
	mEpollEvent.data.fd = pTask->IO()->FD();
	mEpollEvent.data.ptr = pTask;
	int iRet = epoll_ctl(mEpollFD, EPOLL_CTL_ADD, pTask->IO()->FD(), &mEpollEvent);
	if(iRet < 0)
	{
		LOG_ERROR(ELOG_KEY, "[runtime] fd(%d) add into epoll failed: %s", pTask->IO()->FD(), strerror(errno));
		return -1;
	}
	return 0;
}

template <typename T>
int wTcpServer<T>::AddToTaskPool(wTask* pTask)
{
	W_ASSERT(pTask != NULL, return -1;);

	mTaskPool.push_back(pTask);
	//epoll_event大小
	mTaskCount = mTaskPool.size();
	if (mTaskCount > mEpollEventPool.capacity())
	{
		mEpollEventPool.reserve(mTaskCount * 2);
	}
	
	LOG_DEBUG(ELOG_KEY, "[runtime] fd(%d) add into task pool", pTask->IO()->FD());
	return 0;
}

template <typename T>
void wTcpServer<T>::Recv()
{
	int iRet = epoll_wait(mEpollFD, &mEpollEventPool[0], mTaskCount, mTimeout /*10ms*/);
	if(iRet < 0)
	{
		LOG_ERROR(ELOG_KEY, "[runtime] epoll_wait failed: %s", strerror(errno));
		return;
	}
	
	wTask *pTask = NULL;
	for(int i = 0 ; i < iRet ; i++)
	{
		pTask = (wTask *)mEpollEventPool[i].data.ptr;
		
		int iFD = pTask->IO()->FD();
		SOCK_TYPE sockType = pTask->IO()->SockType();

		if(iFD < 0)
		{
			LOG_ERROR(ELOG_KEY, "[runtime] socketfd error fd(%d): %s, close it", iFD, strerror(errno));
			if (RemoveEpoll(pTask) >= 0)
			{
				RemoveTaskPool(pTask);
			}
			continue;
		}
		if (!pTask->IsRunning())	//多数是超时设置
		{
			LOG_ERROR(ELOG_KEY, "[runtime] task status is quit, fd(%d): %s, close it", iFD, strerror(errno));
			if (RemoveEpoll(pTask) >= 0)
			{
				RemoveTaskPool(pTask);
			}
			continue;
		}
		if (mEpollEventPool[i].events & (EPOLLERR | EPOLLPRI))	//出错
		{
			LOG_ERROR(ELOG_KEY, "[runtime] epoll event recv error from fd(%d): %s, close it", iFD, strerror(errno));
			if (RemoveEpoll(pTask) >= 0)
			{
				RemoveTaskPool(pTask);
			}
			continue;
		}
		
		if(sockType == SOCK_LISTEN)
		{
			if (mEpollEventPool[i].events & EPOLLIN)
			{
				AcceptConn();	//accept connect
			}
		}
		else if(sockType == SOCK_CONNECT)
		{
			if (mEpollEventPool[i].events & EPOLLIN)
			{
				//套接口准备好了读取操作
				if (pTask->TaskRecv() <= 0)	//==0主动断开
				{
					LOG_ERROR(ELOG_KEY, "[runtime] EPOLLIN(read) failed or socket closed: %s", strerror(errno));
					if (RemoveEpoll(pTask) >= 0)
					{
						RemoveTaskPool(pTask);
					}
				}
			}
			else if (mEpollEventPool[i].events & EPOLLOUT)
			{
				//套接口准备好了写入操作
				if (pTask->TaskSend() < 0)	//写入失败，半连接
				{
					LOG_ERROR(ELOG_KEY, "[runtime] EPOLLOUT(write) failed: %s", strerror(errno));
					if (RemoveEpoll(pTask) >= 0)
					{
						RemoveTaskPool(pTask);
					}
				}
			}
		}
		else if(sockType == SOCK_UNIX)
		{
			if (mEpollEventPool[i].events & EPOLLIN)
			{
				//channel准备好了读取操作
				if (pTask->TaskRecv() <= 0)	//==0主动断开
				{
					LOG_ERROR(ELOG_KEY, "[runtime] EPOLLIN(read) failed or socket closed: %s", strerror(errno));
					if (RemoveEpoll(pTask) >= 0)
					{
						RemoveTaskPool(pTask);
					}
				}
			}
		}
	}
}

/**
 *  接受新连接
 */
template <typename T>
int wTcpServer<T>::AcceptConn()
{
	if(mListenSock == NULL || mListenSock->IO()->FD() == FD_UNKNOWN)
	{
		return -1;
	}
	int iFD = mListenSock->IO()->FD();
	
	struct sockaddr_in stSockAddr;
	socklen_t iSockAddrSize = sizeof(stSockAddr);
	int iNewFD = accept(iFD, (struct sockaddr*)&stSockAddr, &iSockAddrSize);
	if(iNewFD < 0)
	{
		LOG_ERROR(ELOG_KEY, "[runtime] client connect failed(%s)", strerror(errno));
	    return -1;
    }
	//setsockopt socket：设置发送缓冲大小3M
	int iOptLen = sizeof(socklen_t);
	int iOptVal = 0x300000;
	if(setsockopt(iNewFD, SOL_SOCKET, SO_SNDBUF, (const void *)&iOptVal, iOptLen) < -1)
	{
		LOG_DEBUG(ELOG_KEY, "[runtime] set send buffer size failed");
	}
		
	//new tcp task
	wSocket *pSocket = new wSocket();
	pSocket->FD() = iNewFD;
	pSocket->IPAddr() = inet_ntoa(stSockAddr.sin_addr);
	pSocket->Port() = stSockAddr.sin_port;
	pSocket->SockType() = SOCK_CONNECT;
	pSocket->IOFlag() = FLAG_RVSD;
	pSocket->SockStatus() = STATUS_CONNECTED;
	if (pSocket->SetNonBlock() < 0)
	{
		SAFE_DELETE(pSocket);
		return -1;
	}
	
	mTask = NewTcpTask(pSocket);
	if(NULL != mTask)
	{
		if (mTask->VerifyConn() < 0 || mTask->Verify())
		{
			LOG_ERROR(ELOG_KEY, "[runtime] connect illegal or verify timeout: %d, close it", iNewFD);
			SAFE_DELETE(mTask);
			return -1;
		}
		
		mTask->Status() = TASK_RUNNING;
		if (AddToEpoll(mTask) >= 0)
		{
			AddToTaskPool(mTask);
		}
		LOG_ERROR(ELOG_KEY, "[connect] client connect succeed: ip(%s) port(%d)", pSocket->IPAddr().c_str(), pSocket->Port());
	}
	return iNewFD;
}

template <typename T>
int wTcpServer<T>::RemoveEpoll(wTask* pTask)
{
	int iFD = pTask->IO()->FD();
	mEpollEvent.data.fd = iFD;
	if(epoll_ctl(mEpollFD, EPOLL_CTL_DEL, iFD, &mEpollEvent) < 0)
	{
		LOG_ERROR(ELOG_KEY, "[runtime] epoll remove socket fd(%d) error : %s", iFD, strerror(errno));
		return -1;
	}
	return 0;
}

//返回下一个迭代器
template <typename T>
std::vector<wTask*>::iterator wTcpServer<T>::RemoveTaskPool(wTask* pTask)
{
    std::vector<wTask*>::iterator it = std::find(mTaskPool.begin(), mTaskPool.end(), pTask);
    if(it != mTaskPool.end())
    {
    	SAFE_DELETE(*it);
        it = mTaskPool.erase(it);
    }
    mTaskCount = mTaskPool.size();
    return it;
}

template <typename T>
void wTcpServer<T>::Run()
{
	//...
}

template <typename T>
void wTcpServer<T>::CheckTimer()
{
	unsigned long long iInterval = (unsigned long long)(GetTickCount() - mLastTicker);

	if(iInterval < 100) 	//100ms
	{
		return;
	}

	//加上间隔时间
	mLastTicker += iInterval;

	//检测客户端超时
	if(mCheckTimer.CheckTimer(iInterval))
	{
		CheckTimeout();
	}
}

template <typename T>
void wTcpServer<T>::CheckTimeout()
{
	unsigned long long iNowTime = GetTickCount();
	unsigned long long iIntervalTime;
	
	if(mTaskPool.size() > 0)
	{
		SOCK_TYPE sockType;
		vector<wTask*>::iterator iter;
		for(iter = mTaskPool.begin(); iter != mTaskPool.end(); iter++)
		{
			sockType = (*iter)->IO()->SockType();
			if (sockType == SOCK_CONNECT)
			{
				//心跳检测
				iIntervalTime = iNowTime - (*iter)->IO()->SendTime();	//上一次发送心跳时间间隔
				if(iIntervalTime >= CHECK_CLIENT_TIME)	//3s
				{
					if((*iter)->Heartbeat() < 0 && (*iter)->HeartbeatOutTimes())
					{
						LOG_ERROR(ELOG_KEY, "[runtime] client fd(%d) heartbeat pass limit times, close it", (*iter)->IO()->FD());
						if(RemoveEpoll(*iter) >= 0)
						{
							iter = RemoveTaskPool(*iter);
							iter--;
						}
					}
				}
			}
		}
	}
}

#endif
