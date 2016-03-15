
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

template <typename TASK>
wMTcpClient<TASK>::wMTcpClient()
{
	Initialize();
}

template <typename TASK>
void wMTcpClient<TASK>::Initialize()
{
	mLastTicker = GetTickCount();
	mIsCheckTimer = true;
	mTcpClientCount = 0;
	mTimeout = 10;
	mEpollFD = -1;
	memset((void *)&mEpollEvent, 0, sizeof(mEpollEvent));
	mEpollEventPool.reserve(LISTEN_BACKLOG);
}

template <typename TASK>
wMTcpClient<TASK>::~wMTcpClient()
{
    Final();
}

template <typename TASK>
void wMTcpClient<TASK>::Final()
{
	CleanEpoll();
	CleanTcpClientPool();
}

template <typename TASK>
void wMTcpClient<TASK>::CleanEpoll()
{
	if(mEpollFD != -1)
	{
		close(mEpollFD);
		mEpollFD = -1;
	}
	memset((void *)&mEpollEvent, 0, sizeof(mEpollEvent));
	mEpollEventPool.clear();
}

template <typename TASK>
void wMTcpClient<TASK>::CleanTcpClientPool()
{
    typename map<int, vector<wTcpClient<TASK>*> >::iterator mt = mTcpClientPool.begin();
	for(mt; mt != mTcpClientPool.end(); mt++)
	{
		vector<wTcpClient<TASK>* > vTcpClient = mt->second;
        typename vector<wTcpClient<TASK>*>::iterator vIt = vTcpClient.begin();
		for(vIt; vIt != vTcpClient.end() ; vIt++)
		{
			SAFE_DELETE(*vIt);
		}
	}
	mTcpClientPool.clear();
	mTcpClientCount = 0;
}

template <typename TASK>
void wMTcpClient<TASK>::PrepareStart()
{
	if(InitEpoll() < 0)
	{
		exit(1);
	}
	PrepareRun();
}


template <typename TASK>
void wMTcpClient<TASK>::Start(bool bDaemon)
{
	//进入服务主服务
	do {
		Recv();
		Run();
		if(mIsCheckTimer) CheckTimer();
	} while(bDaemon);
}

template <typename TASK>
void wMTcpClient<TASK>::PrepareRun()
{
	//...
}

template <typename TASK>
void wMTcpClient<TASK>::Run()
{
	//...
}

template <typename TASK>
int wMTcpClient<TASK>::InitEpoll()
{
	mEpollFD = epoll_create(LISTEN_BACKLOG);
	if(mEpollFD < 0)
	{
		mErr = errno;
		LOG_ERROR(ELOG_KEY, "[startup] epoll_create failed:%s", strerror(mErr));
		return -1;
	}
	return mEpollFD;
}

template <typename TASK>
bool wMTcpClient<TASK>::GenerateClient(int iType, string sClientName, char *vIPAddress, unsigned short vPort)
{
	wTcpClient<TASK>* pTcpClient = CreateClient(iType, sClientName, vIPAddress , vPort);
	if(pTcpClient != NULL)
	{
		if (AddToEpoll(pTcpClient) >= 0)
		{
			return AddTcpClientPool(iType, pTcpClient);
		}
	}
	LOG_DEBUG(ELOG_KEY, "[startup] GenerateClient to (%s)server faild!",sClientName.c_str());
	return false;
}

template <typename TASK>
wTcpClient<TASK>* wMTcpClient<TASK>::CreateClient(int iType, string sClientName, char *vIPAddress, unsigned short vPort)
{
	wTcpClient<TASK>* pTcpClient = new wTcpClient<TASK>(iType, sClientName);
	int iRet = pTcpClient->ConnectToServer(vIPAddress, vPort);
	if(iRet >= 0)
	{
		pTcpClient->PrepareStart();	//准备启动
		return pTcpClient;
	}
	LOG_DEBUG(ELOG_KEY, "[startup] connect to (%s)server faild!",sClientName.c_str());
	SAFE_DELETE(pTcpClient);
	return NULL;
}

template <typename TASK>
bool wMTcpClient<TASK>::AddTcpClientPool(int iType, wTcpClient<TASK> *pTcpClient)
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
	
	//客户端总数量
	ResetTcpClientCount();
	return true;
}

template <typename TASK>
int wMTcpClient<TASK>::ResetTcpClientCount()
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
	
	//重置容量
	if (mTcpClientCount > mEpollEventPool.capacity())
	{
		mEpollEventPool.reserve(mTcpClientCount * 2);
	}
	return mTcpClientCount;
}

template <typename TASK>
int wMTcpClient<TASK>::AddToEpoll(wTcpClient<TASK> *pTcpClient, int iEvent)
{
	int iSocketFD = pTcpClient->TcpTask()->IO()->FD();
	mEpollEvent.events = iEvent;	//客户端事件
	mEpollEvent.data.fd = iSocketFD;
	mEpollEvent.data.ptr = pTcpClient;
	int iRet = epoll_ctl(mEpollFD, EPOLL_CTL_ADD, iSocketFD, &mEpollEvent);
	if(iRet < 0)
	{
		LOG_ERROR(ELOG_KEY, "[runtime] fd(%d) add into epoll failed: %s", iSocketFD, strerror(errno));
		return -1;
	}
	return 0;
}

//心跳
template <typename TASK>
void wMTcpClient<TASK>::CheckTimer()
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

template <typename TASK>
void wMTcpClient<TASK>::Recv()
{
	int iRet = epoll_wait(mEpollFD, &mEpollEventPool[0], mTcpClientCount, mTimeout /*10ms*/);
	if(iRet < 0)
	{
		LOG_ERROR(ELOG_KEY, "[runtime] epoll_wait failed: %s", strerror(errno));
		return;
	}
	
	wTcpClient<TASK> *pClient = NULL;
	wTcpTask *pTask = NULL;
	int iType = 0;

	int iFD = FD_UNKNOWN;
	SOCK_TYPE sockType;
	SOCK_STATUS sockStatus;
	IO_FLAG iOFlag;
	for(int i = 0 ; i < iRet ; i++)
	{
		pClient = (wTcpClient<TASK>*) mEpollEventPool[i].data.ptr;
		pTask = pClient->TcpTask();
		iType = pClient->Type();
		
		iFD = pTask->IO()->FD();
		sockType = pTask->IO()->SockType();
		sockStatus = pTask->IO()->SockStatus();
		iOFlag = pTask->IO()->IOFlag();
		if(iFD < 0)
		{
			mErr = errno;
			LOG_ERROR(ELOG_KEY, "[runtime] socketfd error fd(%d): %s, close it", iFD, strerror(mErr));
			//LOG_ERROR("server", "[disconnect] socketfd error(%s): ip(%s) port(%d)", strerror(mErr), pTask->IO()->Host().c_str(), pTask->IO()->Port());
			if (RemoveEpoll(pClient) >= 0)
			{
				RemoveTcpClientPool(iType, pClient);
			}
			continue;
		}
		if (!pClient->IsRunning() || !pTask->IsRunning())	//多数是超时设置
		{
			LOG_ERROR(ELOG_KEY, "[runtime] task status is quit, fd(%d), close it", iFD);
			if (RemoveEpoll(pClient) >= 0)
			{
				RemoveTcpClientPool(iType, pClient);
			}
			continue;
		}

		if (mEpollEventPool[i].events & (EPOLLERR | EPOLLPRI))
		{
			mErr = errno;
			LOG_ERROR(ELOG_KEY, "[runtime] epoll event recv error from fd(%d): %s, close it", iFD, strerror(mErr));
			//LOG_ERROR("server", "[disconnect] epoll event recv error(%s): ip(%s) port(%d)", strerror(mErr), pTask->IO()->Host().c_str(), pTask->IO()->Port());
			if (RemoveEpoll(pClient) >= 0)
			{
				RemoveTcpClientPool(iType, pClient);
			}
			continue;
		}

		if(sockType == SOCK_CONNECT)	//connect event: read|write
		{
			if (mEpollEventPool[i].events & EPOLLIN)
			{
				//套接口准备好了读取操作
				if (pTask->TaskRecv() <= 0)
				{
					mErr = errno;
					LOG_ERROR(ELOG_KEY, "[runtime] EPOLLIN(read) failed or server-socket closed: %s", strerror(mErr));
					//LOG_ERROR("server", "[disconnect] EPOLLIN(read) failed or socket closed(%s):ip(%s) port(%d)", strerror(mErr), pTask->IO()->Host().c_str(), pTask->IO()->Port());
					if (RemoveEpoll(pClient) >= 0)
					{
						RemoveTcpClientPool(iType, pClient);
					}
				}
			}
			else if (mEpollEventPool[i].events & EPOLLOUT)
			{
				//套接口准备好了写入操作
				if (pTask->TaskSend() < 0)
				{
					mErr = errno;
					LOG_ERROR(ELOG_KEY, "[runtime] EPOLLOUT(write) failed: %s", strerror(mErr));
					//LOG_ERROR("server", "[disconnect] EPOLLOUT(write) failed(%s): ip(%s) port(%d)", strerror(mErr), pTask->IO()->Host().c_str(), pTask->IO()->Port());
					if (RemoveEpoll(pClient) >= 0)
					{
						RemoveTcpClientPool(iType, pClient);
					}
				}
			}
		}
	}
}

template <typename TASK>
int wMTcpClient<TASK>::RemoveEpoll(wTcpClient<TASK> *pTcpClient)
{
	int iSocketFD = pTcpClient->TcpTask()->IO()->FD();
	mEpollEvent.data.fd = iSocketFD;
	if(epoll_ctl(mEpollFD, EPOLL_CTL_DEL, iSocketFD, &mEpollEvent) < 0)
	{
		LOG_ERROR("error", "[runtime] epoll remove socket fd(%d) error : %s", iSocketFD, strerror(errno));
		return -1;
	}
	return 0;
}

template <typename TASK>
bool wMTcpClient<TASK>::RemoveTcpClientPool(int iType, wTcpClient<TASK> *pTcpClient)
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

template <typename TASK>
vector<wTcpClient<TASK>*> wMTcpClient<TASK>::TcpClients(int iType)
{
	vector<wTcpClient<TASK>*> vTcpClient;
    typename map<int, vector<wTcpClient<TASK>*> >::iterator mt = mTcpClientPool.find(iType);
	if(mt != mTcpClientPool.end())
	{
		vTcpClient = mt->second;
	}
	return vTcpClient;
}

template <typename TASK>
wTcpClient<TASK>* wMTcpClient<TASK>::OneTcpClient(int iType)
{
	vector<wTcpClient<TASK>*> vTcpClient;
    typename map<int, vector<wTcpClient<TASK>*> >::iterator mt = mTcpClientPool.find(iType);
	if(mt != mTcpClientPool.end())
	{
		vTcpClient = mt->second;
		return vTcpClient[0];	//第一个连接
	}
	return NULL;
}
