
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_MTCP_CLIENT_H_
#define _W_MTCP_CLIENT_H_

#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <algorithm>
#include <string>
#include <string.h>
#include <map>
#include <vector>

#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include "wType.h"
#include "wTimer.h"
#include "wMisc.h"
#include "wLog.h"
#include "wSingleton.h"
#include "wTcpClient.h"

template <typename T,typename TASK>
class wMTcpClient: public wSingleton<T>
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
		
		void Recv();
		void Send();
		
		bool IsRunning()
		{
			return CLIENT_STATUS_RUNNING == mStatus;
		}
		
		void SetStatus(CLIENT_STATUS eStatus = CLIENT_STATUS_QUIT)
		{
			mStatus = eStatus;
		}
		
		CLIENT_STATUS GetStatus()
		{
			return mStatus;
		}

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

        std::map<int, vector<wTcpClient<TASK>*> > mTcpClientPool;	//每种类型客户端，可挂载多个连接
};

template <typename T,typename TASK>
wMTcpClient<T,TASK>::wMTcpClient()
{
	mStatus = CLIENT_STATUS_INIT;
	Initialize();
}

template <typename T,typename TASK>
void wMTcpClient<T,TASK>::Initialize()
{
	mLastTicker = GetTickCount();
	mClientCount = 0;
	mTimeout = 10;
	mEpollFD = -1;
	memset((void *)&mEpollEvent, 0, sizeof(mEpollEvent));
	mEpollEventPool.reserve(512);	//容量
}

template <typename T,typename TASK>
wMTcpClient<T,TASK>::~wMTcpClient()
{
    Final();
}

template <typename T,typename TASK>
void wMTcpClient<T,TASK>::Final()
{
	CleanEpoll();
	CleanTcpClientPool();
}

template <typename T,typename TASK>
void wMTcpClient<T,TASK>::CleanEpoll()
{
	if(mEpollFD != -1)
	{
		close(mEpollFD);
	}
	mEpollFD = -1;
	memset((void *)&mEpollEvent, 0, sizeof(mEpollEvent));
	mEpollEventPool.clear();
}

template <typename T,typename TASK>
void wMTcpClient<T,TASK>::CleanTcpClientPool()
{
    typename map<int, vector<wTcpClient<TASK>*> >::iterator mt = mTcpClientPool.begin();
	for(mt; mt != mTcpClientPool.end(); mt++)
	{
		vector<wTcpClient<TASK>* > vTcpClient = mt->second;
        typename vector<wTcpClient<TASK>*>::iterator vIt = vTcpClient.begin();
		for(vIt ; vIt != vTcpClient.end() ; vIt++)
		{
			SAFE_DELETE(*vIt);
		}
	}
	mTcpClientPool.clear();
	mTcpClientCount = 0;
}

template <typename T,typename TASK>
void wMTcpClient<T,TASK>::PrepareStart()
{
	SetStatus(CLIENT_STATUS_RUNNING);
	if(InitEpoll() < 0)
	{
		exit(1);
	}
	PrepareRun();
}

template <typename T,typename TASK>
void wMTcpClient<T,TASK>::PrepareRun()
{
	//...
}

template <typename T,typename TASK>
int wMTcpClient<T,TASK>::InitEpoll()
{
	mEpollFD = epoll_create(512); //512
	if(mEpollFD < 0)
	{
		LOG_ERROR("default", "epoll_create failed: %s", strerror(errno));
		return -1;
	}
	return mEpollFD;
}

template <typename T,typename TASK>
bool wMTcpClient<T,TASK>::GenerateClient(int iType, string sClientName, char *vIPAddress, unsigned short vPort)
{
	wTcpClient<TASK>* pTcpClient = CreateClient(iType, sClientName, vIPAddress , vPort);
	if(pTcpClient != NULL)
	{
		if (AddToEpoll(pTcpClient) >= 0)
		{
			return AddTcpClientPool(iType, pTcpClient);
		}
	}
	LOG_DEBUG("default", "GenerateClient to (%s)server faild!",sClientName.c_str());
	return false;
}

template <typename T,typename TASK>
wTcpClient<TASK>* wMTcpClient<T,TASK>::CreateClient(int iType, string sClientName, char *vIPAddress, unsigned short vPort)
{
	wTcpClient<TASK>* pTcpClient = new wTcpClient<TASK>(iType, sClientName);
	int iRet = pTcpClient->ConnectToServer(vIPAddress, vPort);
	if(iRet >= 0)
	{
		pTcpClient->PrepareStart();
		return pTcpClient;
	}
	LOG_DEBUG("default", "connect to (%s)server faild!",sClientName.c_str());
	SAFE_DELETE(pTcpClient);
	return NULL;
}

template <typename T,typename TASK>
bool wMTcpClient<T,TASK>::AddTcpClientPool(int iType, wTcpClient<TASK> *pTcpClient)
{
	if(pTcpClient == NULL)
	{
		return false;
	}
	vector<wTcpClient<TASK>*> vTcpClient;
    typename map<int, vector<wTcpClient<TASK>*> >::iterator mt = mTcpClientPool.find(iType);
	if(mt != mTcpClientPool.end())
	{
		vTcpClient = mt->second;
		mTcpClientPool.erase(mt);
	}
	vTcpClient.push_back(pTcpClient);
	mTcpClientPool.insert(pair<int, vector<wTcpClient<TASK>*> >(iType, vTcpClient));
	
	ResetTcpClientCount();
	return true;
}

template <typename T,typename TASK>
int wMTcpClient<T,TASK>::ResetTcpClientCount()
{
	mTcpClientCount = 0;
    typename map<int, vector<wTcpClient<TASK>*> >::iterator mt = mTcpClientPool.begin();
	for(mt; mt != mTcpClientPool.end(); mt++)
	{
		vector<wTcpClient<TASK>* > vTcpClient = mt->second;
        int iType = mt->first;
        typename vector<wTcpClient<TASK>*>::iterator vIt = vTcpClient.begin();
		for(vIt ; vIt != vTcpClient.end() ; vIt++)
		{
			mTcpClientCount++;
		}
	}
	
	if (mTcpClientCount > mEpollEventPool.capacity())
	{
		mEpollEventPool.reserve(mTcpClientCount * 2);
	}
	return mTcpClientCount;
}

template <typename T,typename TASK>
int wMTcpClient<T,TASK>::AddToEpoll(wTcpClient<TASK> *pTcpClient)
{
	int iSocketFD = pTcpClient->TcpTask()->Socket()->SocketFD();
	mEpollEvent.events = EPOLLIN | EPOLLERR | EPOLLHUP;	//客户端事件
	mEpollEvent.data.fd = iSocketFD;
	mEpollEvent.data.ptr = pTcpClient;
	int iRet = epoll_ctl(mEpollFD, EPOLL_CTL_ADD, iSocketFD, &mEpollEvent);
	if(iRet < 0)
	{
		LOG_ERROR("default", "fd(%d) add into epoll failed: %s", iSocketFD, strerror(errno));
		return -1;
	}
	return 0;
}

template <typename T,typename TASK>
void wMTcpClient<T,TASK>::Start(bool bDaemon)
{
	//进入服务主服务
	do
	{
		Recv();
		
		Run();
		
		CheckTimer();
	} while(IsRunning() && bDaemon);
}

template <typename T,typename TASK>
void wMTcpClient<T,TASK>::CheckTimer()
{
    typename map<int, vector<wTcpClient<TASK>*> >::iterator mt = mTcpClientPool.begin();
	for(mt; mt != mTcpClientPool.end(); mt++)
	{
		vector<wTcpClient<TASK>* > vTcpClient = mt->second;
		int iType = mt->first;
        typename vector<wTcpClient<TASK>*>::iterator vIt = vTcpClient.begin();
		for(vIt ; vIt != vTcpClient.end() ; vIt++)
		{
			if((*vIt)->IsRunning())
			{
				(*vIt)->CheckTimer();
			}
			else
			{
				if (RemoveEpoll(*vIt) >= 0)
				{
					RemoveTcpClientPool(iType, *vIt);
					vIt--;
				}
			}
		}
	}
}

template <typename T,typename TASK>
void wMTcpClient<T,TASK>::Recv()
{
	int iRet = epoll_wait(mEpollFD, &mEpollEventPool[0], mTcpClientCount, mTimeout /*10ms*/);
	if(iRet < 0)
	{
		LOG_ERROR("default", "epoll_wait failed: %s", strerror(errno));
		return;
	}
	for(int i = 0 ; i < iRet ; i++)
	{
		wTcpClient<TASK> *pClient = (wTcpClient<TASK> *)mEpollEventPool[i].data.ptr;
		int iType = pClient->GetType();
		wTcpTask* pTask = pClient->TcpTask();
		
		if(pTask->Socket()->SocketFD() < 0)
		{
			LOG_ERROR("default", "socketfd error fd(%d): %s, close it", pTask->Socket()->SocketFD(), strerror(errno));
			if (RemoveEpoll(pClient) >= 0)
			{
				RemoveTcpClientPool(iType, pClient);
			}
			continue;
		}
		if (mEpollEventPool[i].events & (EPOLLERR | EPOLLPRI))
		{
			LOG_ERROR("default", "epoll event recv error from fd(%d): %s, close it", pTask->Socket()->SocketFD(), strerror(errno));
			if (RemoveEpoll(pClient) >= 0)
			{
				RemoveTcpClientPool(iType, pClient);
			}
			continue;
		}

		if(pTask->Socket()->SocketType() == CONNECT_SOCKET)	//connect event: read|write
		{
			if (mEpollEventPool[i].events & EPOLLIN)
			{
				//套接口准备好了读取操作
				if (pTask->ListeningRecv() <= 0)
				{
					LOG_ERROR("default", "EPOLLIN(read) failed or server-socket closed: %s", strerror(errno));
					if (RemoveEpoll(pClient) >= 0)
					{
						RemoveTcpClientPool(iType, pClient);
					}
				}
			}
			else if (mEpollEventPool[i].events & EPOLLOUT)
			{
				//套接口准备好了写入操作
				if (pTask->ListeningSend() < 0)
				{
					LOG_ERROR("default", "EPOLLOUT(write) failed: %s", strerror(errno));
					if (RemoveEpoll(pClient) >= 0)
					{
						RemoveTcpClientPool(iType, pClient);
					}
				}
			}
		}
	}
}

template <typename T>
int wMTcpClient<T,TASK>::RemoveEpoll(wTcpClient<TASK> *pTcpClient)
{
	int iSocketFD = pTcpClient->TcpTask()->Socket()->SocketFD();
	mEpollEvent.data.fd = iSocketFD;
	if(epoll_ctl(mEpollFD, EPOLL_CTL_DEL, iSocketFD, &mEpollEvent) < 0)
	{
		LOG_ERROR("default", "epoll remove socket fd(%d) error : %s", iSocketFD, strerror(errno));
		return -1;
	}
	return 0;
}

template <typename T,typename TASK>
bool wMTcpClient<T,TASK>::RemoveTcpClientPool(int iType, wTcpClient<TASK> *pTcpClient)
{
	typename map<int, vector<wTcpClient<TASK>*> >::iterator mt = mTcpClientPool.find(iType);
	if(mt != mTcpClientPool.end())
	{
		vector<wTcpClient<TASK>*> vTcpClient = mt->second;
		if(pTcpClient == NULL)
		{
			typename vector<wTcpClient<TASK>*>::iterator it = vTcpClient.begin();
			for(it; it != vTcpClient.end(); it++)
			{
				SAFE_DELETE(*it);
			}
			mTcpClientPool.erase(mt);
			return true;
		}
		else
		{
			typename vector<wTcpClient<TASK>*>::iterator it = find(vTcpClient.begin(), vTcpClient.end(), pTcpClient);
			if(it != vTcpClient.end())
			{
				vTcpClient.erase(it);
				SAFE_DELETE(*it);
				mTcpClientPool.erase(mt);
				mTcpClientPool.insert(pair<int, vector<wTcpClient<TASK>*> >(iType, vTcpClient));
				return true;
			}
			return false;
		}
	}
	return false;
}

template <typename T,typename TASK>
void wMTcpClient<T,TASK>::Run()
{
	//...
}

template <typename T,typename TASK>
vector<wTcpClient<TASK>*> wMTcpClient<T,TASK>::TcpClients(int iType)
{
	vector<wTcpClient<TASK>*> vTcpClient;
    typename map<int, vector<wTcpClient<TASK>*> >::iterator mt = mTcpClientPool.find(iType);
	if(mt != mTcpClientPool.end())
	{
		vTcpClient = mt->second;
	}
	return vTcpClient;
}

template <typename T,typename TASK>
wTcpClient<TASK>* wMTcpClient<T,TASK>::OneTcpClient(int iType)
{
	vector<wTcpClient<TASK>*> vTcpClient;
    typename map<int, vector<wTcpClient<TASK>*> >::iterator mt = mTcpClientPool.find(iType);
	if(mt != mTcpClientPool.end())
	{
		vTcpClient = mt->second;
		return vTcpClient[0];
	}
	return NULL;
}

#endif
