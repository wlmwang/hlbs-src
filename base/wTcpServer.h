
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_TCP_SERVER_H_
#define _W_TCP_SERVER_H_

#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <algorithm>
#include <string>
#include <string.h>
#include <vector>

#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include "wType.h"
#include "wMisc.h"
#include "wLog.h"
#include "wSingleton.h"
#include "wSocket.h"
#include "wTcpTask.h"

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
		void Send();
		
		/**
		 * 初始化服务器
		 */
		void Initialize();

		/**
		 * 准备|启动服务
		 */
		void PrepareStart(string sIpAddr ,unsigned int nPort);
		void Start();
		
		/**
		 * epoll|socket相关
		 */
		void InitEpoll();
		void CleanEpoll();
		int AddToEpoll(wTcpTask* vTcpTask);
        int RemoveEpoll(wTcpTask* stTcpTask);

		void InitListen(string sIpAddr ,unsigned int nPort);
		
		/**
		 * 接受|新建客户端
		 */
		virtual wTcpTask* NewTcpTask(wSocket *pSocket);
		void AcceptConn(int iSocketFD);
		void AddToTaskPool(wTcpTask* stTcpTask);
		void CleanTaskPool();
	    void RemoveTaskPool(wTcpTask* stTcpTask);

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
		
	protected:
        wTcpServer(string ServerName, int vInitFlag = true);
		//禁用复制函数
		wTcpServer(const wTcpServer&);
		
		SERVER_STATUS mStatus;	//服务器当前状态
		//定时记录器
		unsigned long long mLastTicker;	//服务器当前时间
		
	protected:
		string mServerName;
		
		wSocket *mSocket;	//Listen Socket(主服务socket对象)
		
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
	mServerName = "";
	mSocket = NULL;
	mTaskCount = 0;
	mTimeout = 10;
	mEpollFD = -1;
	memset((void *)&mEpollEvent, 0, sizeof(mEpollEvent));
	mEpollEventPool.resize(512);	//epoll事件池默认大小
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
	if(!mTcpTaskPool.empty())
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
void wTcpServer<T>::PrepareStart(string sIpAddr ,unsigned int nPort)
{
	LOG_INFO("default", "Server Prepare start succeed");
	
	//初始化epoll
	InitEpoll();
	
	//初始化Listen Socket
	InitListen(sIpAddr ,nPort);

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
void wTcpServer<T>::InitEpoll()
{
	mEpollFD = epoll_create(512); //512
	if(mEpollFD < 0)
	{
		LOG_ERROR("default", "epoll_create failed: %s", strerror(errno));
		exit(1);
	}
}

template <typename T>
void wTcpServer<T>::InitListen(string sIpAddr ,unsigned int nPort)
{
	int iSocketFD = socket(AF_INET, SOCK_STREAM, 0); 
	if(iSocketFD < 0)
	{
		LOG_ERROR("default", "Create socket failed: %s", strerror(errno));
		exit(1);
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
		close(iSocketFD);
		printf("Socket bind failed\n");
		exit(1);
	}
	//setsockopt socket : 设置发送缓冲大小4M
	int iOptLen = sizeof(socklen_t);
	int iOptVal = 0x400000;
	if(setsockopt(iSocketFD, SOL_SOCKET, SO_SNDBUF, (const void *)&iOptVal, iOptLen) < -1)
	{
		close(iSocketFD);
		LOG_INFO("default", "set send buffer size failed");
		exit(1);
	}
	if(getsockopt(iSocketFD, SOL_SOCKET, SO_SNDBUF, (void *)&iOptVal, (socklen_t *)&iOptLen) >= 0)
	{
		LOG_DEBUG("default", "set send buffer to %d", iOptVal);
	}

	//listen
	iRet = listen(iSocketFD, 512);
	if(iRet < 0)
	{
		LOG_ERROR("default", "listen failed: %s", strerror(errno));
		exit(1);
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
		if(mTcpTask->Socket()->SetNonBlock() < 0) 
		{
			LOG_ERROR("default", "set non block failed: %d, close it", iSocketFD);
			SAFE_DELETE(mTcpTask);
			return ;
		}
		if (AddToEpoll(mTcpTask) >= 0)
		{
			AddToTaskPool(mTcpTask);
		}
	}
}

template <typename T>
void wTcpServer<T>::Start()
{
	mStatus = SERVER_STATUS_RUNNING;
	LOG_INFO("default", "Server start succeed");
	//进入服务主循环
	while(IsRunning())
	{
		//接受客户端消息
		Recv();
		
		Run();
	}
}

/**
 *  接受新连接
 */
template <typename T>
void wTcpServer<T>::AcceptConn(int iSocketFD)
{
	struct sockaddr_in stSockAddr;
	socklen_t iSockAddrSize = sizeof(stSockAddr);
	int iNewSocketFD = accept(iSocketFD, (struct sockaddr*)&stSockAddr, &iSockAddrSize);
	if(iNewSocketFD < 0)
	{
		LOG_INFO("default", "client connect port and disconnected");
	    return;
    }
	//setsockopt socket：设置发送缓冲大小3M
	int iOptLen = sizeof(socklen_t);
	int iOptVal = 0x300000;
	if(setsockopt(iNewSocketFD, SOL_SOCKET, SO_SNDBUF, (const void *)&iOptVal, iOptLen) < -1)
	{
		LOG_INFO("default", "set send buffer size failed");
	}
	else
	{
		if(getsockopt(iNewSocketFD, SOL_SOCKET, SO_SNDBUF, (void *)&iOptVal, (socklen_t *)&iOptLen) >= 0 )
		{
			LOG_DEBUG("default", "set send buffer to %d", iOptVal);
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
		if(mTcpTask->Socket()->SetNonBlock() < 0) 
		{
			LOG_ERROR("default", "set non block failed: %d, close it", iNewSocketFD);
			SAFE_DELETE(mTcpTask);
			return ;
		}
		if (AddToEpoll(mTcpTask) >= 0)
		{
			AddToTaskPool(mTcpTask);
		}
	}
}

template <typename T>
wTcpTask* wTcpServer<T>::NewTcpTask(wSocket *pSocket)
{
	wTcpTask *pTcpTask = new wTcpTask(pSocket);
	return pTcpTask;
}

/**
 * 添加到epoll监听事件中
 * @param  stTcpTask [wTcpTask*]
 * @return           [是否出错]
 */
template <typename T>
int wTcpServer<T>::AddToEpoll(wTcpTask* stTcpTask)
{
	mEpollEvent.events = EPOLLIN | EPOLLERR | EPOLLHUP;	//客户端事件
	mEpollEvent.data.fd = stTcpTask->Socket()->SocketFD();
	mEpollEvent.data.ptr = stTcpTask;
	int iRet = epoll_ctl(mEpollFD, EPOLL_CTL_ADD, stTcpTask->Socket()->SocketFD(), &mEpollEvent);
	if(iRet < 0)
	{
		LOG_ERROR("default", "fd(%d) add into epoll failed: %s", stTcpTask->Socket()->SocketFD(), strerror(errno));
		return -1;
	}
	return 0;
}

template <typename T>
void wTcpServer<T>::AddToTaskPool(wTcpTask* stTcpTask)
{
	//TcpTask池
	mTcpTaskPool.push_back(stTcpTask);

	//epoll_event大小
	mTaskCount = mTcpTaskPool.size();
	if (mTaskCount > mEpollEventPool.size())
	{
		mEpollEventPool.resize(mTaskCount + 16);
	}
}

template <typename T>
void wTcpServer<T>::Recv()
{
	//接受消息写入到消息队列中去（共享队列）
	int iRet = epoll_wait(mEpollFD, &mEpollEventPool[0], mTaskCount, mTimeout);
	if(iRet < 0)
	{
		LOG_ERROR("default", "epoll_wait failed: %s", strerror(errno));
		return;
	}
	for(int i = 0 ; i < iRet ; i++)
	{
		wTcpTask *pTask = (wTcpTask *)mEpollEventPool[i].data.ptr;
		
		if(pTask->Socket()->SocketFD() >0)
		{
			if (RemoveEpoll(pTask))
			{
				RemoveTaskPool(pTask);
			}
		}
		if (mEpollEventPool[i].events & (EPOLLERR | EPOLLPRI))
		{
			LOG_ERROR("default", "epoll event recv error from fd(%d): %s, close it", pTask->Socket()->SocketFD(), strerror(errno));
			if (RemoveEpoll(pTask))
			{
				RemoveTaskPool(pTask);
			}
		}
		
		if(pTask->Socket()->SocketType() == LISTEN_SOCKET)
		{
			AcceptConn(pTask->Socket()->SocketFD());
		}
		else if(pTask->Socket()->SocketType() == CONNECT_SOCKET)
		{
			if (mEpollEventPool[i].events & EPOLLIN)
			{
				//套接口准备好了读取操作
				if (pTask->ListeningRecv() < 0)
				{
					LOG_ERROR("default", "EPOLLIN(read) failed: %s", strerror(errno));
					if (RemoveEpoll(pTask))
					{
						RemoveTaskPool(pTask);
					}
				}
			}
			else if (mEpollEventPool[i].events & EPOLLOUT)
			{
				//套接口准备好了写入操作
				if (pTask->ListeningSend() < 0)
				{
					LOG_ERROR("default", "EPOLLOUT(write) failed: %s", strerror(errno));
					if (RemoveEpoll(pTask))
					{
						RemoveTaskPool(pTask);
					}
				}
			}
		}
	}
}

template <typename T>
int wTcpServer<T>::RemoveEpoll(wTcpTask* pTcpTask)
{
	int iSocketFD = pTcpTask->Socket()->SocketFD();
	mEpollEvent.data.fd = iSocketFD;
	if(epoll_ctl(mEpollFD, EPOLL_CTL_DEL, iSocketFD, &mEpollEvent) < 0)
	{
		LOG_ERROR("default", "epoll remove socket fd(%d) error : %s", iSocketFD, strerror(errno));
		return -1;
	}
	return 0;
}

template <typename T>
void wTcpServer<T>::RemoveTaskPool(wTcpTask* pTcpTask)
{
    std::vector<wTcpTask*>::iterator it = std::find(mTcpTaskPool.begin(), mTcpTaskPool.end(), pTcpTask);
    if(it != mTcpTaskPool.end())
    {
        mTcpTaskPool.erase(it);
		SAFE_DELETE(*it);
    }
    mTaskCount = mTcpTaskPool.size();
}

template <typename T>
void wTcpServer<T>::Run()
{
	//...
}

#endif
