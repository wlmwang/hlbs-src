
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_TCP_SERVER_H_
#define _W_TCP_SERVER_H_

#include <algorithm>
#include <vector>

#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "wAssert.h"
#include "wType.h"
#include "wMisc.h"
#include "wLog.h"
#include "wSingleton.h"
#include "wSocket.h"
#include "wTcpTask.h"
#include "wTimer.h"

enum SERVER_STATUS
{
	SERVER_STATUS_INIT,				//服务器的初始化状态
	SERVER_STATUS_QUIT,				//服务器进入关闭状态
	SERVER_STATUS_RUNNING			//正常运行状态模式
};

template <typename T>
class wTcpServer: public wSingleton<T>
{
	public:
		void Recv();
		void Broadcast(const char *pCmd, int iLen);
		
		/**
		 * 初始化服务器
		 */
		void Initialize();

		/**
		 * 准备|启动服务
		 */
		void PrepareStart(string sIpAddr ,unsigned int nPort);
		void Start(bool bDaemon = true);
		
		/**
		 * epoll|socket相关
		 */
		int InitEpoll();
		void CleanEpoll();
		int AddToEpoll(wTcpTask* vTcpTask);
        int RemoveEpoll(wTcpTask* stTcpTask);

		int InitListen(string sIpAddr ,unsigned int nPort);
		
		/**
		 * 接受|新建客户端
		 */
		virtual wTcpTask* NewTcpTask(wSocket *pSocket);
		int AcceptConn();
		int AddToTaskPool(wTcpTask* stTcpTask);
		void CleanTaskPool();
	    std::vector<wTcpTask*>::iterator RemoveTaskPool(wTcpTask* stTcpTask);

		/**
		 * 服务主循环逻辑，继承可以定制服务
		 */
		virtual void PrepareRun();
		virtual void Run();
		
		/**
		 * 检查服务器运行状态
		 */
		bool IsRunning()
		{
			return SERVER_STATUS_RUNNING == mStatus;
		}
		
		/**
		 *  设置服务器状态
		 */
		void SetStatus(SERVER_STATUS eStatus = SERVER_STATUS_QUIT)
		{
			mStatus = eStatus;
		}
		
		/**
		 *  获取服务器状态
		 */
		SERVER_STATUS GetStatus()
		{
			return mStatus;
		}
		
		/**
		 *  服务器名
		 */
		string GetServerName()
		{
			return mServerName;
		}
		
		void Final();
		
		virtual ~wTcpServer();
		
		void CheckTimer();
		virtual void CheckTimeout();
	protected:
        wTcpServer(string ServerName, int vInitFlag = true);
		wTcpServer(const wTcpServer&);
		
		string mServerName;
		wSocket *mSocket;	//Listen Socket(主服务socket对象)
		
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
		wTcpTask *mTcpTask;
		vector<wTcpTask*> mTcpTaskPool;
};

/** 模板函数实现 */
template <typename T>
wTcpServer<T>::wTcpServer(string ServerName, int vInitFlag)
{
	if(vInitFlag)
	{
		mStatus = SERVER_STATUS_INIT;
		Initialize();
	}
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
	mSocket = NULL;
	mTaskCount = 0;
	mTimeout = 10;
	mEpollFD = -1;
	memset((void *)&mEpollEvent, 0, sizeof(mEpollEvent));
	mEpollEventPool.reserve(512);	//容量
}

template <typename T>
void wTcpServer<T>::Final()
{
	CleanEpoll();
	CleanTaskPool();
	SAFE_DELETE(mSocket);
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
	if(mTcpTaskPool.size() > 0)
	{
		vector<wTcpTask*>::iterator iter;
		for(iter = mTcpTaskPool.begin(); iter != mTcpTaskPool.end(); iter++)
		{
			SAFE_DELETE(*iter);
		}
	}
	mTcpTaskPool.clear();
	mTaskCount = 0;
}

template <typename T>
void wTcpServer<T>::Broadcast(const char *pCmd, int iLen)
{
	if(mTcpTaskPool.size() > 0)
	{
		vector<wTcpTask*>::iterator iter;
		for(iter = mTcpTaskPool.begin(); iter != mTcpTaskPool.end(); iter++)
		{
			if ((*iter)->IsRunning() && (*iter)->Socket()->SocketType() == CONNECT_SOCKET && (*iter)->Socket()->SocketFlag() == RECV_DATA)
			{
				(*iter)->SyncSend(pCmd, iLen);
			}
		}
	}
}

template <typename T>
void wTcpServer<T>::PrepareStart(string sIpAddr ,unsigned int nPort)
{
	LOG_INFO("default", "[startup] Server Prepare start succeed");
	
	//初始化epoll
	if(InitEpoll() < 0)
	{
		LOG_ERROR("error", "[startup] InitEpoll failed: %s", strerror(errno));
		exit(1);
	}
	
	//初始化Listen Socket
	if(InitListen(sIpAddr ,nPort) < 0)
	{
		LOG_ERROR("error", "[startup] InitListen failed: %s", strerror(errno));
		exit(1);
	}

	//运行前工作
	PrepareRun();
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
		LOG_ERROR("error", "[startup] epoll_create failed: %s", strerror(errno));
		return -1;
	}
	return mEpollFD;
}

template <typename T>
int wTcpServer<T>::InitListen(string sIpAddr ,unsigned int nPort)
{
	int iSocketFD = socket(AF_INET, SOCK_STREAM, 0); 
	if(iSocketFD < 0)
	{
		LOG_ERROR("error", "[startup] Create socket failed: %s", strerror(errno));
		return -1;
	}
	//setsockopt socket
	int iFlags = 1;
	struct linger stLing = {0,0};
	setsockopt(iSocketFD, SOL_SOCKET, SO_REUSEADDR, &iFlags, sizeof(iFlags));
	setsockopt(iSocketFD, SOL_SOCKET, SO_KEEPALIVE, &iFlags, sizeof(iFlags));
	setsockopt(iSocketFD, SOL_SOCKET, SO_LINGER, &stLing, sizeof(stLing));

	//bind socket
	struct sockaddr_in stSocketAddr;
	stSocketAddr.sin_family = AF_INET;
	stSocketAddr.sin_port = htons((short)nPort);
	stSocketAddr.sin_addr.s_addr = inet_addr(sIpAddr.c_str());

	int iRet = bind(iSocketFD, (struct sockaddr *)&stSocketAddr, sizeof(stSocketAddr));
	if(iRet < 0)
	{
		LOG_ERROR("error", "[startup] Socket bind failed");
		close(iSocketFD);
		return -1;
	}
	//setsockopt socket : 设置发送缓冲大小4M
	int iOptLen = sizeof(socklen_t);
	int iOptVal = 0x400000;
	if(setsockopt(iSocketFD, SOL_SOCKET, SO_SNDBUF, (const void *)&iOptVal, iOptLen) < -1)
	{
		LOG_ERROR("error", "[startup] Set send buffer size failed");
		close(iSocketFD);
		return -1;
	}
	if(getsockopt(iSocketFD, SOL_SOCKET, SO_SNDBUF, (void *)&iOptVal, (socklen_t *)&iOptLen) >= 0)
	{
		LOG_DEBUG("default", "[startup] Set send buffer to %d", iOptVal);
	}

	//listen
	int backlog = 512;
	iRet = listen(iSocketFD, backlog);
	if(iRet < 0)
	{
		LOG_ERROR("error", "[startup] listen failed: %s", strerror(errno));
		return -1;
	}
	
	//add epoll event
	mSocket = new wSocket();
	mSocket->SocketFD() = iSocketFD;
	mSocket->IPAddr() = sIpAddr;
	mSocket->Port() = nPort;
	mSocket->SocketType() = LISTEN_SOCKET;
	mSocket->SocketFlag() = RECV_DATA;
	
	mTcpTask = NewTcpTask(mSocket);
	if(NULL != mTcpTask)
	{
		mTcpTask->SetStatus(SOCKET_STATUS_RUNNING);
		if(mTcpTask->Socket()->SetNonBlock() < 0) 
		{
			LOG_ERROR("error", "[startup] Set non block failed: %d, close it", iSocketFD);
			SAFE_DELETE(mTcpTask);
			return -1;
		}
		if (AddToEpoll(mTcpTask) >= 0)
		{
			AddToTaskPool(mTcpTask);
		}
	}
	return iSocketFD;
}

template <typename T>
wTcpTask* wTcpServer<T>::NewTcpTask(wSocket *pSocket)
{
	wTcpTask *pTcpTask = new wTcpTask(pSocket);
	return pTcpTask;
}

/**
 * 添加到epoll监听事件中
 * @param  pTcpTask [wTcpTask*]
 * @return           [是否出错]
 */
template <typename T>
int wTcpServer<T>::AddToEpoll(wTcpTask* pTcpTask)
{
	mEpollEvent.events = EPOLLIN | EPOLLERR | EPOLLHUP;	//客户端事件
	mEpollEvent.data.fd = pTcpTask->Socket()->SocketFD();
	mEpollEvent.data.ptr = pTcpTask;
	int iRet = epoll_ctl(mEpollFD, EPOLL_CTL_ADD, pTcpTask->Socket()->SocketFD(), &mEpollEvent);
	if(iRet < 0)
	{
		LOG_ERROR("error", "[runtime] fd(%d) add into epoll failed: %s", pTcpTask->Socket()->SocketFD(), strerror(errno));
		return -1;
	}
	return 0;
}

template <typename T>
int wTcpServer<T>::AddToTaskPool(wTcpTask* pTcpTask)
{
	W_ASSERT(pTcpTask != NULL, return -1;);

	mTcpTaskPool.push_back(pTcpTask);

	//epoll_event大小
	mTaskCount = mTcpTaskPool.size();
	if (mTaskCount > mEpollEventPool.capacity())
	{
		mEpollEventPool.reserve(mTaskCount * 2);
	}
	//LOG_INFO("default", "[runtime] fd(%d) add into task pool", pTcpTask->Socket()->SocketFD());
	return 0;
}

template <typename T>
void wTcpServer<T>::Start(bool bDaemon)
{
	mStatus = SERVER_STATUS_RUNNING;
	LOG_INFO("default", "[startup] Server start succeed");
	//进入服务主循环
	do {
		Recv();
		Run();
		if(mIsCheckTimer) CheckTimer();
	} while(IsRunning() && bDaemon);
}

template <typename T>
void wTcpServer<T>::Recv()
{
	int iRet = epoll_wait(mEpollFD, &mEpollEventPool[0], mTaskCount, mTimeout /*10ms*/);
	if(iRet < 0)
	{
		LOG_ERROR("error", "[runtime] epoll_wait failed: %s", strerror(errno));
		return;
	}
	for(int i = 0 ; i < iRet ; i++)
	{
		wTcpTask *pTask = (wTcpTask *)mEpollEventPool[i].data.ptr;
		int iSocketFD = pTask->Socket()->SocketFD();
		int iSocketType = pTask->Socket()->SocketType();

		if(iSocketFD < 0)
		{
			LOG_ERROR("error", "[runtime] socketfd error fd(%d): %s, close it", iSocketFD, strerror(errno));
			LOG_ERROR("client", "[disconnect] socketfd error(%s): ip(%s) port(%d)", strerror(errno), pTask->Socket()->IPAddr().c_str(), pTask->Socket()->Port());
			if (RemoveEpoll(pTask) >= 0)
			{
				RemoveTaskPool(pTask);
			}
			continue;
		}
		if (!pTask->IsRunning())	//多数是超时设置
		{
			LOG_ERROR("error", "[runtime] task status is quit, fd(%d): %s, close it", iSocketFD, strerror(errno));
			LOG_ERROR("client", "[disconnect] task status is quit(%s): ip(%s) port(%d)", strerror(errno), pTask->Socket()->IPAddr().c_str(), pTask->Socket()->Port());
			if (RemoveEpoll(pTask) >= 0)
			{
				RemoveTaskPool(pTask);
			}
			continue;
		}
		if (mEpollEventPool[i].events & (EPOLLERR | EPOLLPRI))	//出错
		{
			LOG_ERROR("error", "[runtime] epoll event recv error from fd(%d): %s, close it", iSocketFD, strerror(errno));
			LOG_ERROR("client", "[disconnect] epoll event recv error(%s): ip(%s) port(%d)", strerror(errno), pTask->Socket()->IPAddr().c_str(), pTask->Socket()->Port());
			if (RemoveEpoll(pTask) >= 0)
			{
				RemoveTaskPool(pTask);
			}
			continue;
		}

		if(iSocketType == LISTEN_SOCKET)
		{
			if (mEpollEventPool[i].events & EPOLLIN)
			{
				AcceptConn();	//accept connect
			}
		}
		else if(iSocketType == CONNECT_SOCKET)	//connect event: read|write
		{
			if (mEpollEventPool[i].events & EPOLLIN)
			{
				//套接口准备好了读取操作
				if (pTask->ListeningRecv() <= 0)	//==0主动断开
				{
					LOG_ERROR("error", "[runtime] EPOLLIN(read) failed or socket closed: %s", strerror(errno));
					LOG_ERROR("client", "[disconnect] EPOLLIN(read) failed or socket closed(%s):ip(%s) port(%d)", strerror(errno), pTask->Socket()->IPAddr().c_str(), pTask->Socket()->Port());
					if (RemoveEpoll(pTask) >= 0)
					{
						RemoveTaskPool(pTask);
					}
				}
			}
			else if (mEpollEventPool[i].events & EPOLLOUT)
			{
				//套接口准备好了写入操作
				if (pTask->ListeningSend() < 0)	//写入失败，半连接
				{
					LOG_ERROR("error", "[runtime] EPOLLOUT(write) failed: %s", strerror(errno));
					LOG_ERROR("client", "[disconnect] EPOLLOUT(write) failed(%s): ip(%s) port(%d)", strerror(errno), pTask->Socket()->IPAddr().c_str(), pTask->Socket()->Port());
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
	if(mSocket == NULL || mSocket->SocketFD() < 0)
	{
		return -1;
	}
	struct sockaddr_in stSockAddr;
	socklen_t iSockAddrSize = sizeof(stSockAddr);
	int iNewSocketFD = accept(mSocket->SocketFD(), (struct sockaddr*)&stSockAddr, &iSockAddrSize);
	if(iNewSocketFD < 0)
	{
		LOG_ERROR("error", "[runtime] client connect failed(%s)", strerror(errno));
	    return -1;
    }
	//setsockopt socket：设置发送缓冲大小3M
	int iOptLen = sizeof(socklen_t);
	int iOptVal = 0x300000;
	if(setsockopt(iNewSocketFD, SOL_SOCKET, SO_SNDBUF, (const void *)&iOptVal, iOptLen) < -1)
	{
		LOG_DEBUG("error", "[runtime] set send buffer size failed");
	}
	else
	{
		if(getsockopt(iNewSocketFD, SOL_SOCKET, SO_SNDBUF, (void *)&iOptVal, (socklen_t *)&iOptLen) >= 0 )
		{
			LOG_DEBUG("default", "[runtime] set send buffer to %d", iOptVal);
		}
	}
	
	//new tcp task
	wSocket *pSocket = new wSocket();
	pSocket->SocketFD() = iNewSocketFD;
	pSocket->IPAddr() = inet_ntoa(stSockAddr.sin_addr);
	pSocket->Port() = stSockAddr.sin_port;
	pSocket->SocketType() = CONNECT_SOCKET;
	pSocket->SocketFlag() = RECV_DATA;
		
	mTcpTask = NewTcpTask(pSocket);
	if(NULL != mTcpTask)
	{
		if (mTcpTask->VerifyConn() < 0 || mTcpTask->Verify())
		{
			LOG_ERROR("error", "[runtime] connect illegal or verify timeout: %d, close it", iNewSocketFD);
			SAFE_DELETE(mTcpTask);
			return -1;
		}
		
		mTcpTask->SetStatus(SOCKET_STATUS_RUNNING);
		if(mTcpTask->Socket()->SetNonBlock() < 0) 
		{
			LOG_ERROR("error", "[runtime] set non block failed: %d, close it", iNewSocketFD);
			SAFE_DELETE(mTcpTask);
			return -1;
		}
		if (AddToEpoll(mTcpTask) >= 0)
		{
			AddToTaskPool(mTcpTask);
		}
		LOG_ERROR("client", "[connect] client connect succeed: ip(%s) port(%d)", mTcpTask->Socket()->IPAddr().c_str(), mTcpTask->Socket()->Port());
	}
	return iNewSocketFD;
}

template <typename T>
int wTcpServer<T>::RemoveEpoll(wTcpTask* pTcpTask)
{
	int iSocketFD = pTcpTask->Socket()->SocketFD();
	mEpollEvent.data.fd = iSocketFD;
	if(epoll_ctl(mEpollFD, EPOLL_CTL_DEL, iSocketFD, &mEpollEvent) < 0)
	{
		LOG_ERROR("error", "[runtime] epoll remove socket fd(%d) error : %s", iSocketFD, strerror(errno));
		return -1;
	}
	return 0;
}

//返回下一个迭代器
template <typename T>
std::vector<wTcpTask*>::iterator wTcpServer<T>::RemoveTaskPool(wTcpTask* pTcpTask)
{
    std::vector<wTcpTask*>::iterator it = std::find(mTcpTaskPool.begin(), mTcpTaskPool.end(), pTcpTask);
    if(it != mTcpTaskPool.end())
    {
    	SAFE_DELETE(*it);
        it = mTcpTaskPool.erase(it);
    }
    mTaskCount = mTcpTaskPool.size();
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
	
	if(mTcpTaskPool.size() > 0)
	{
		vector<wTcpTask*>::iterator iter;
		for(iter = mTcpTaskPool.begin(); iter != mTcpTaskPool.end(); iter++)
		{
			if ((*iter)->Socket()->SocketType() != CONNECT_SOCKET)
			{
				continue;
			}
			//心跳检测
			iIntervalTime = iNowTime - (*iter)->Socket()->SendTime();	//上一次发送心跳时间间隔
			if(iIntervalTime >= CHECK_CLIENT_TIME)	//3s
			{
				if((*iter)->Heartbeat() < 0 && (*iter)->HeartbeatOutTimes())
				{
					LOG_ERROR("error", "[runtime] client ip(%s) fd(%d) heartbeat pass limit times, close it", (*iter)->Socket()->IPAddr().c_str(), (*iter)->Socket()->SocketFD());
					LOG_ERROR("client", "[timeout] client heartbeat pass limit times: ip(%s) port(%d)", (*iter)->Socket()->IPAddr().c_str(), (*iter)->Socket()->Port());
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

#endif
