
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
#include <string.h>
#include <string>
#include <vector>

#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include "wType.h"
#include "wMisc.h"
#include "wLog.h"
#include "wSingleton.h"
#include "wTcpTask.h"
#include "wSocket.h"

enum SERVER_STATUS
{
	SERVER_STATUS_INIT,				//服务器的初始化状态
	SERVER_STATUS_QUIT,				//服务器进入关闭状态
	SERVER_STATUS_RUNNING			//正常运行状态模式
};

template <typename T>
class wTcpServer: public wSocket, public wSingleton<T>
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
		void PrepareStart(string sIpAddr ,unsigned int nPort ,unsigned int nBacklog = 512);
		void Start();
		
		/**
		 * epoll|socket相关
		 */
		void InitEpoll();
		void CleanEpoll();
		int AddToEpoll(wTcpTask* vTcpTask);
        int RemoveEpoll(wTcpTask* stTcpTask);

		void InitListen();
		
		/**
		 * 接受|新建客户端
		 */
		virtual wTcpTask* NewTcpTask(int iNewSocket, struct sockaddr_in* stSockAddr);
		void AcceptConn();
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

	//private:
		
        //epoll
		int mEpollFD;
		int mTimeout;	//epoll_wait timeout
		
		unsigned int mBacklog; //socket backlog(listen)
		
		//connect socket
		struct sockaddr_in mSockAddr;	//connect socket(accept)
		
		//epoll_event
		struct epoll_event mEpollEvent;
		vector<struct epoll_event> mEpollEventPool; //epoll_event已发生事件池（epoll_wait）
		int mTaskCount;	//mTcpTaskPool.size();
		
		//task|pool
		wTcpTask *mTcpTask;
		vector<wTcpTask*> mTcpTaskPool;
		
		string mServerName;
};

/** 模板函数实现 */
template <typename T>
wTcpServer<T>::wTcpServer(string ServerName, int vInitFlag)
{
	mServerName = ServerName;
	if(vInitFlag)
	{
		mStatus = SERVER_STATUS_INIT;
		Initialize();
	}
	else
	{
		//mStatus = SERVER_STATUS_RESUME;
	}
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
	Close();
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
			(*iter)->Close();
			SAFE_DELETE(*iter);
		}
	}
	mTcpTaskPool.clear();
	mTaskCount = 0;
}

template <typename T>
void wTcpServer<T>::PrepareStart(string sIpAddr ,unsigned int nPort ,unsigned int nBacklog)
{
	LOG_INFO("default", "Server Prepare start succeed");
	
	mIPAddr = sIpAddr;	//监听地址
	mPort = nPort;	//监听端口
	mBacklog = nBacklog;	//accept队列
	
	//初始化epoll
	InitEpoll();
	
	//初始化Listen Socket
	InitListen();

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
void wTcpServer<T>::InitListen()
{
	mSocketFD = socket(AF_INET, SOCK_STREAM, 0); 
	if(mSocketFD < 0)
	{
		LOG_ERROR("default", "Create socket failed: %s", strerror(errno));
		exit(1);
	}

	//setsockopt socket
	int iFlags = 1;
	struct linger stLing = {0,0};
	setsockopt(mSocketFD, SOL_SOCKET, SO_REUSEADDR, &iFlags, sizeof(iFlags));
	setsockopt(mSocketFD, SOL_SOCKET, SO_KEEPALIVE, &iFlags, sizeof(iFlags));
	setsockopt(mSocketFD, SOL_SOCKET, SO_LINGER, &stLing, sizeof(stLing));

	//bind socket
	struct sockaddr_in stSocketAddr;
	stSocketAddr.sin_family = AF_INET;
	stSocketAddr.sin_port = htons((short)mPort);
	stSocketAddr.sin_addr.s_addr = inet_addr(mIPAddr.c_str());

	int iRet = bind(mSocketFD, (struct sockaddr *)&stSocketAddr, sizeof(stSocketAddr));
	if(iRet < 0)
	{
		Close();
		printf("Socket bind failed\n");
		exit(1);
	}
	
	//setsockopt socket : 设置发送缓冲大小4M
	int iOptLen = sizeof(socklen_t);
	int iOptVal = 0x400000;
	if(setsockopt(mSocketFD, SOL_SOCKET, SO_SNDBUF, (const void *)&iOptVal, iOptLen) < -1)
	{
		LOG_INFO("default", "set send buffer size failed");
		exit(1);
	}
	if(getsockopt(mSocketFD, SOL_SOCKET, SO_SNDBUF, (void *)&iOptVal, (socklen_t *)&iOptLen) >= 0)
	{
		LOG_DEBUG("default", "set send buffer to %d", iOptVal);
	}

	//listen
	iRet = listen(mSocketFD, mBacklog);
	if(iRet < 0)
	{
		LOG_ERROR("default", "listen failed: %s", strerror(errno));
		exit(1);
	}
}

template <typename T>
void wTcpServer<T>::Start()
{
	mStatus = SERVER_STATUS_RUNNING;
	LOG_INFO("default", "Server start succeed");
	//进入服务主循环
	while(mStatus == SERVER_STATUS_RUNNING && IsRunning() && mSocketFD != -1)
	{
		//接受客户端连接
		AcceptConn();
		
		//接受客户端消息
		Recv();
		
		//定制服务
		Run();
	}
}

/**
 *  接受新连接
 */
template <typename T>
void wTcpServer<T>::AcceptConn()
{
	socklen_t iSockAddrSize = sizeof(mSockAddr);
	int iNewSocket = accept(mSocketFD, (struct sockaddr*)&mSockAddr, &iSockAddrSize);
	if(iNewSocket < 0)
	{
		LOG_INFO("default", "client connect port and disconnected");
	    return;
    }
	
	//setsockopt socket：设置发送缓冲大小3M
	int iOptLen = sizeof(socklen_t);
	int iOptVal = 0x300000;
	if(setsockopt(mSocketFD, SOL_SOCKET, SO_SNDBUF, (const void *)&iOptVal, iOptLen) < -1)
	{
		LOG_INFO("default", "set send buffer size failed");
	}
	else
	{
		if(getsockopt(mSocketFD, SOL_SOCKET, SO_SNDBUF, (void *)&iOptVal, (socklen_t *)&iOptLen) >= 0 )
		{
			LOG_DEBUG("default", "set send buffer to %d", iOptVal);
		}
	}
	
	//new tcp task
	mTcpTask = NewTcpTask(iNewSocket, &mSockAddr);

	if(NULL != mTcpTask)
	{
		if(mTcpTask->SetNonBlock() < 0) 
		{
			LOG_ERROR("default", "set non block failed: %d, close it", iNewSocket);
			mTcpTask->Close();
		}
		if (AddToEpoll(mTcpTask) >= 0)
		{
			AddToTaskPool(mTcpTask);
		}
	}
}

template <typename T>
wTcpTask* wTcpServer<T>::NewTcpTask(int iNewSocket, struct sockaddr_in *stSockAddr)
{
	wTcpTask *pTcpTask = new wTcpTask(iNewSocket, stSockAddr);
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
	mEpollEvent.data.fd = stTcpTask->mSocketFD;
	mEpollEvent.data.ptr = stTcpTask;
	int iRet = epoll_ctl(mEpollFD, EPOLL_CTL_ADD, stTcpTask->mSocketFD, &mEpollEvent);
	if(iRet < 0)
	{
		LOG_ERROR("default", "fd(%d) add into epoll failed: %s", stTcpTask->mSocketFD, strerror(errno));
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
	int iRet = epoll_wait(mEpollFD, &mEpollEventPool[0], mTaskCount, -1 /*mTimeout*/);
	if(iRet < 0)
	{
		LOG_ERROR("default", "epoll_wait failed: %s", strerror(errno));
		return;
	}
	socklen_t iSockAddrSize = sizeof(mSockAddr);
	
	for(int i = 0 ; i < iRet ; i++)
	{
		wTcpTask *vTask = (wTcpTask *)mEpollEventPool[i].data.ptr;	//TcpTask对象
		if (mEpollEventPool[i].events & (EPOLLERR | EPOLLPRI))
		{
			LOG_ERROR("default", "epoll event recv error from fd(%d): %s, close it", vTask->mSocketFD, strerror(errno));
			if (RemoveEpoll(vTask))
			{
				RemoveTaskPool(vTask);
				//vTask->Terminate(CLOSE_REASON vReason);
			}
		}
		else
		{
			if (mEpollEventPool[i].events & EPOLLIN)
			{
				//套接口准备好了读取操作
				if (vTask->ListeningRecv() < 0)
				{	
					LOG_ERROR("default", "EPOLLIN(read) failed: %s", strerror(errno));
					if (RemoveEpoll(vTask))
					{
						RemoveTaskPool(vTask);
						//vTask->Terminate(CLOSE_REASON vReason);
					}
				}
			}
			if (mEpollEventPool[i].events & EPOLLOUT)
			{
				//套接口准备好了写入操作
				if (vTask->ListeningSend() < 0)
				{
					LOG_ERROR("default", "EPOLLOUT(write) failed: %s", strerror(errno));
					if (RemoveEpoll(vTask))
					{
						RemoveTaskPool(vTask);
						//vTask->Terminate(CLOSE_REASON vReason);
					}
				}
			}
		}
	}
}

template <typename T>
int wTcpServer<T>::RemoveEpoll(wTcpTask* stTcpTask)
{
	int iSocketFD = stTcpTask->mSocketFD;
	mEpollEvent.data.fd = iSocketFD;
	if(epoll_ctl(mEpollFD, EPOLL_CTL_DEL, iSocketFD, &mEpollEvent) < 0)
	{
		LOG_ERROR("default", "epoll remove socket fd(%d) error : %s", iSocketFD, strerror(errno));
		return -1;
	}
	stTcpTask->Close();
	return 0;
}

template <typename T>
void wTcpServer<T>::RemoveTaskPool(wTcpTask* stTcpTask)
{
    std::vector<wTcpTask*>::iterator itPosition = std::find(mTcpTaskPool.begin(), mTcpTaskPool.end(), stTcpTask);
    if(itPosition != mTcpTaskPool.end())
    {
    	(*itPosition)->Close();
        mTcpTaskPool.erase(itPosition);
    }
    mTaskCount = mTcpTaskPool.size();
}

template <typename T>
void wTcpServer<T>::Run()
{
	//...
}

#endif
