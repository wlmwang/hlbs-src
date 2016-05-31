
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

template <typename T>
wServer<T>::wServer(string ServerName)
{
	Initialize();
	
	mStatus = SERVER_INIT;
	mServerName = ServerName;
}

template <typename T>
wServer<T>::~wServer()
{
	Final();
}

template <typename T>
void wServer<T>::Initialize()
{
	mTimeout = 10;
	mServerName = "";

	mLastTicker = GetTickCount();
	mCheckTimer = wTimer(KEEPALIVE_TIME);
	mIsCheckTimer = false;	//不开启心跳机制

	mEpollFD = FD_UNKNOWN;
	memset((void *)&mEpollEvent, 0, sizeof(mEpollEvent));
	mTaskCount = 0;
	mEpollEventPool.reserve(LISTEN_BACKLOG);	//容量
	mTask = NULL;

	mListenSock = NULL;
	mChannelSock = NULL;
	mWorker = NULL;
	mExiting = 0;
}

template <typename T>
void wServer<T>::PrepareSingle(string sIpAddr, unsigned int nPort)
{
	LOG_INFO(ELOG_KEY, "[system] Server Prepare start succeed");
	
	//初始化epoll
	if (InitEpoll() < 0)
	{
		exit(0);
	}
	
	//初始化Listen Socket
	if (InitListen(sIpAddr ,nPort) < 0)
	{
		exit(0);
	}

	//Listen Socket 添加到epoll中
	if (mListenSock == NULL)
	{
		exit(0);
	}
	mTask = NewTcpTask(mListenSock);
	if(NULL != mTask)
	{
		mTask->Status() = TASK_RUNNING;
		if (AddToEpoll(mTask) >= 0)
		{
			AddToTaskPool(mTask);
		}
		else
		{
			mTask->DeleteIO();
			SAFE_DELETE(mTask);
			exit(1);
		}
	}
	
	//运行前工作
	PrepareRun();
}

template <typename T>
void wServer<T>::SingleStart(bool bDaemon)
{
	LOG_INFO(ELOG_KEY, "[system] Server start succeed");
	
	mStatus = SERVER_RUNNING;	
	//进入服务主循环
	do {
		Recv();
		Run();
		CheckTimer();
	} while(IsRunning() && bDaemon);
}

template <typename T>
void wServer<T>::PrepareMaster(string sIpAddr, unsigned int nPort)
{
	LOG_INFO(ELOG_KEY, "[system] listen socket on ip(%s) port(%d)", sIpAddr.c_str(), nPort);
	
	//初始化Listen Socket
	if (InitListen(sIpAddr ,nPort) < 0)
	{
		exit(0);
	}

	//运行前工作
	PrepareRun();
}

template <typename T>
void wServer<T>::WorkerStart(wWorker *pWorker, bool bDaemon)
{
	LOG_INFO(ELOG_KEY, "[system] worker start succeed");
	
	mStatus = SERVER_RUNNING;
	//初始化epoll
	if (InitEpoll() < 0)
	{
		exit(2);
	}
	
	//Listen Socket 添加到epoll中
	if (mListenSock == NULL)
	{
		exit(2);
	}
	mTask = NewTcpTask(mListenSock);
	if (NULL != mTask)
	{
		mTask->Status() = TASK_RUNNING;
		if (AddToEpoll(mTask) >= 0)
		{
			AddToTaskPool(mTask);
		}
		else
		{
			mTask->DeleteIO();
			SAFE_DELETE(mTask);
			exit(2);
		}
	}

	//Unix Socket 添加到epoll中（worker自身channel[1]被监听）
	if (pWorker != NULL)
	{
		mWorker = pWorker;
		
		if (mWorker->mWorkerPool != NULL)
		{
			mChannelSock = &mWorker->mWorkerPool[mWorker->mSlot]->mCh;	//当前worker进程表项
			if (mChannelSock != NULL)
			{
				//new unix task
				mChannelSock->IOFlag() = FLAG_RECV;
				
				mTask = NewChannelTask(mChannelSock);
				if (NULL != mTask)
				{
					mTask->Status() = TASK_RUNNING;
					if (AddToEpoll(mTask) >= 0)
					{
						AddToTaskPool(mTask);
					}
					else
					{
						SAFE_DELETE(mTask);
						exit(2);
					}
				}
			}
			else
			{
				LOG_ERROR(ELOG_KEY, "[system] worker pool slot(%d) illegal", mWorker->mSlot);
				exit(2);
			}
		}
	}
	
	//进入服务主循环
	do {
		if (mExiting)	//退出
		{
			LOG_ERROR(ELOG_KEY, "[system] worker exiting");
			WorkerExit();
		}
		Recv();
		HandleSignal();
		Run();
		CheckTimer();
	} while (IsRunning() && bDaemon);
}

template <typename T>
void wServer<T>::HandleSignal()
{	
	if (g_terminate)	//直接退出
	{
		LOG_ERROR(ELOG_KEY, "[system] worker exiting");
		WorkerExit();
	}
	
	if (g_quit)		//平滑退出
	{
		g_quit = 0;

		LOG_ERROR(ELOG_KEY, "[system] gracefully shutting down");
		if (!mExiting)	//关闭listen socket、idle connect socket
		{
			mExiting = 1;
			mListenSock->Close();
		}
	}
}

template <typename T>
void wServer<T>::WorkerExit()
{
	if (mExiting)	//关闭池
	{
		//
	}

	LOG_ERROR(ELOG_KEY, "[system] worker exit");
	exit(0);
}

template <typename T>
void wServer<T>::PrepareRun()
{
	//accept前准备工作
}

/**
 * epoll初始化
 */
template <typename T>
int wServer<T>::InitEpoll()
{
	mEpollFD = epoll_create(LISTEN_BACKLOG); //511
	if (mEpollFD < 0)
	{
		mErr = errno;
		LOG_ERROR(ELOG_KEY, "[system] epoll_create failed:%s", strerror(mErr));
		return -1;
	}
	return mEpollFD;
}

template <typename T>
int wServer<T>::InitListen(string sIpAddr ,unsigned int nPort)
{
	mListenSock = new wSocket();
	int iFD = mListenSock->Open();
	if (iFD == -1)
	{
		LOG_ERROR(ELOG_KEY, "[system] listen socket open failed");
		SAFE_DELETE(mListenSock);
		return -1;
	}
	
	//listen socket
	if(mListenSock->Listen(sIpAddr, nPort) < 0)
	{
		mErr = errno;
		LOG_ERROR(ELOG_KEY, "[system] listen failed: %s", strerror(mErr));
		SAFE_DELETE(mListenSock);
		return -1;
	}
	
	//nonblock
	if(mListenSock->SetNonBlock() < 0) 
	{
		LOG_ERROR(ELOG_KEY, "[system] Set non block failed: %d, close it", iFD);
		SAFE_DELETE(mListenSock);
		return -1;
	}
	
	mListenSock->SockStatus() = STATUS_LISTEN;	
	return iFD;
}

template <typename T>
wTask* wServer<T>::NewTcpTask(wIO *pIO)
{
	wTask *pTask = new wTcpTask(pIO);
	return pTask;
}

template <typename T>
wTask* wServer<T>::NewHttpTask(wIO *pIO)
{
	wTask *pTask = new wHttpTask(pIO);
	return pTask;
}

template <typename T>
wTask* wServer<T>::NewChannelTask(wIO *pIO)
{
	wTask *pTask = new wChannelTask(pIO);
	return pTask;
}

template <typename T>
int wServer<T>::AddToEpoll(wTask* pTask, int iEvents, int iOp)
{
	mEpollEvent.events = iEvents | EPOLLERR | EPOLLHUP; //|EPOLLET
	mEpollEvent.data.fd = pTask->IO()->FD();
	mEpollEvent.data.ptr = pTask;
	int iRet = epoll_ctl(mEpollFD, iOp, pTask->IO()->FD(), &mEpollEvent);
	if (iRet < 0)
	{
		mErr = errno;
		LOG_ERROR(ELOG_KEY, "[system] fd(%d) add into epoll failed: %s", pTask->IO()->FD(), strerror(mErr));
		return -1;
	}
	LOG_DEBUG(ELOG_KEY, "[system] %s fd %d events read %d write %d", iOp == EPOLL_CTL_MOD ? "mod":"add", pTask->IO()->FD(), mEpollEvent.events & EPOLLIN, mEpollEvent.events & EPOLLOUT);
	return 0;
}

template <typename T>
int wServer<T>::AddToTaskPool(wTask* pTask)
{
	W_ASSERT(pTask != NULL, return -1);

	mTaskPool.push_back(pTask);
	//epoll_event大小
	mTaskCount = mTaskPool.size();
	if (mTaskCount > mEpollEventPool.capacity())
	{
		mEpollEventPool.reserve(mTaskCount * 2);
	}
	LOG_DEBUG(ELOG_KEY, "[system] fd(%d) add into task pool", pTask->IO()->FD());
	return 0;
}

template <typename T>
int wServer<T>::AcceptMutexLock()
{
	return 0;
	if (mWorker != NULL && mWorker->mMutex)
	{
		if (mWorker->mMutexHeld == 1)
		{
			return 0;
		}
		if (mWorker->mMutex->TryLock() == 0)
		{
			mWorker->mMutexHeld = 1;
			return 0;
		}
		return -1;
	}
	return 0;
}

template <typename T>
int wServer<T>::AcceptMutexUnlock()
{
	return 0;
	if (mWorker != NULL && mWorker->mMutex)
	{
		if (mWorker->mMutexHeld == 0)
		{
			return 0;
		}
		if (mWorker->mMutex->Unlock() == 0)
		{
			mWorker->mMutexHeld = 0;
			return 0;
		}
		return -1;
	}
	return 0;
}

template <typename T>
void wServer<T>::Recv()
{
	//申请惊群锁
	if (AcceptMutexLock() != 0)
	{
		return;
	}

	int iRet = epoll_wait(mEpollFD, &mEpollEventPool[0], mTaskCount, mTimeout /*10ms*/);
	if (iRet < 0)
	{
		mErr = errno;
		LOG_ERROR(ELOG_KEY, "[system] epoll_wait failed: %s", strerror(mErr));
		return;
	}
	
	int iFD = FD_UNKNOWN;
	int iLenOrErr;
	wTask *pTask = NULL;
	TASK_TYPE taskType;
	SOCK_TYPE sockType;
	IO_FLAG iOFlag;
	IO_TYPE iOType;
	for (int i = 0 ; i < iRet ; i++)
	{
		pTask = (wTask *)mEpollEventPool[i].data.ptr;
		iFD = pTask->IO()->FD();
		taskType = pTask->IO()->TaskType();
		sockType = pTask->IO()->SockType();
		iOFlag = pTask->IO()->IOFlag();
		iOType = pTask->IO()->IOType();
		
		if (iFD == FD_UNKNOWN)
		{
			LOG_DEBUG(ELOG_KEY, "[system] socket FD is error, fd(%d), close it", iFD);
			if (RemoveEpoll(pTask) >= 0)
			{
				RemoveTaskPool(pTask);
			}
			continue;
		}
		if (!pTask->IsRunning())	//多数是超时设置
		{
			LOG_DEBUG(ELOG_KEY, "[system] task status is quit, fd(%d), close it", iFD);
			if (RemoveEpoll(pTask) >= 0)
			{
				RemoveTaskPool(pTask);
			}
			continue;
		}
		if (mEpollEventPool[i].events & (EPOLLERR | EPOLLPRI))	//出错(多数为sock已关闭)
		{
			mErr = errno;
			LOG_ERROR(ELOG_KEY, "[system] epoll event recv error from fd(%d), close it: %s", iFD, strerror(mErr));
			if (RemoveEpoll(pTask) >= 0)
			{
				RemoveTaskPool(pTask);
			}
			continue;
		}
		
		//tcp|http|unix
		if (iOType == TYPE_SOCK && sockType == SOCK_LISTEN)
		{
			if (mEpollEventPool[i].events & EPOLLIN)
			{
				AcceptConn();	//accept connect
			}
		}
		else if (iOType == TYPE_SOCK && sockType == SOCK_CONNECT)
		{
			if (mEpollEventPool[i].events & EPOLLIN)
			{
				//套接口准备好了读取操作
				if ((iLenOrErr = pTask->TaskRecv()) < 0)
				{
					if (iLenOrErr == ERR_CLOSED)
					{
						LOG_DEBUG(ELOG_KEY, "[system] tcp socket closed by client");
					}
					else if(iLenOrErr == ERR_MSGLEN)
					{
						LOG_ERROR(ELOG_KEY, "[system] recv message invalid len");
					}
					else
					{
						LOG_ERROR(ELOG_KEY, "[system] EPOLLIN(read) failed or tcp socket closed: %s", strerror(pTask->IO()->Errno()));
					}
					if (RemoveEpoll(pTask) >= 0)
					{
						RemoveTaskPool(pTask);
					}
				}
			}
			else if (mEpollEventPool[i].events & EPOLLOUT)
			{
				//清除写事件
				if (pTask->WritableLen() <= 0)
				{
					AddToEpoll(pTask, EPOLLIN, EPOLL_CTL_MOD);
					continue;
				}
				//套接口准备好了写入操作
				if (pTask->TaskSend() < 0)	//写入失败，半连接，对端读关闭
				{
					LOG_ERROR(ELOG_KEY, "[system] EPOLLOUT(write) failed or tcp socket closed: %s", strerror(pTask->IO()->Errno()));
					if (RemoveEpoll(pTask) >= 0)
					{
						RemoveTaskPool(pTask);
					}
				}
			}
		}
	}

	//释放惊群锁
	AcceptMutexUnlock();
}

template <typename T>
int wServer<T>::Send(wTask *pTask, const char *pCmd, int iLen)
{
	W_ASSERT(pTask != NULL, return -1);

	if (pTask->IsRunning() && pTask->IO()->IOType() == TYPE_SOCK && (pTask->IO()->IOFlag() == FLAG_SEND || pTask->IO()->IOFlag() == FLAG_RVSD))
	{
		if (pTask->SendToBuf(pCmd, iLen) > 0)
		{
			return AddToEpoll(pTask, EPOLLIN | EPOLLOUT, EPOLL_CTL_MOD);
		}
	}
	return -1;
}

/** 只广播tcp连接 */
template <typename T>
void wServer<T>::Broadcast(const char *pCmd, int iLen)
{
	W_ASSERT(pCmd != NULL, return);

	if(mTaskPool.size() > 0)
	{
		vector<wTask*>::iterator iter;
		for(iter = mTaskPool.begin(); iter != mTaskPool.end(); iter++)
		{
			if (*iter != NULL && (*iter)->IO()->TaskType() == TASK_TCP)
			{
				Send(*iter, pCmd, iLen);
			}
		}
	}
}

/**
 *  接受新连接
 */
template <typename T>
int wServer<T>::AcceptConn()
{
	if(mListenSock == NULL)
	{
		return -1;
	}
	
	struct sockaddr_in stSockAddr;
	socklen_t iSockAddrSize = sizeof(stSockAddr);	
	int iNewFD = mListenSock->Accept((struct sockaddr*)&stSockAddr, &iSockAddrSize);
	if (iNewFD <= 0)
	{
		if (iNewFD < 0)
		{
			LOG_ERROR(ELOG_KEY, "[system] client connect failed:%s", strerror(mListenSock->Errno()));
		}
	    return iNewFD;
    }
		
	//new tcp task
	wSocket *pSocket = new wSocket();
	pSocket->FD() = iNewFD;
	pSocket->Host() = inet_ntoa(stSockAddr.sin_addr);
	pSocket->Port() = stSockAddr.sin_port;
	pSocket->SockType() = SOCK_CONNECT;
	pSocket->IOFlag() = FLAG_RVSD;
	pSocket->TaskType() = TASK_TCP;
	pSocket->SockStatus() = STATUS_CONNECTED;
	if (pSocket->SetNonBlock() < 0)
	{
		SAFE_DELETE(pSocket);
		return -1;
	}
	
	mTask = NewTcpTask(pSocket);
	if(NULL != mTask)
	{
		if(mTask->VerifyConn() < 0 || mTask->Verify())
		{
			LOG_ERROR(ELOG_KEY, "[system] connect illegal or verify timeout: %d, close it", iNewFD);
			mTask->DeleteIO();
			SAFE_DELETE(mTask);
			return -1;
		}
		
		mTask->Status() = TASK_RUNNING;
		if(AddToEpoll(mTask) >= 0)
		{
			AddToTaskPool(mTask);
		}
		else
		{
			mTask->DeleteIO();
			SAFE_DELETE(mTask);
			return -1;
		}
		LOG_DEBUG(ELOG_KEY, "[system] client connect succeed: ip(%s) port(%d)", pSocket->Host().c_str(), pSocket->Port());
	}
	return iNewFD;
}

template <typename T>
void wServer<T>::Final()
{
	CleanEpoll();
	CleanTaskPool();
}

template <typename T>
void wServer<T>::CleanEpoll()
{
	if (mEpollFD != -1)
	{
		close(mEpollFD);
	}
	mEpollFD = -1;
	memset((void *)&mEpollEvent, 0, sizeof(mEpollEvent));
	mEpollEventPool.clear();
}

template <typename T>
void wServer<T>::CleanTaskPool()
{
	if (mTaskPool.size() > 0)
	{
		vector<wTask*>::iterator it;
		for (it = mTaskPool.begin(); it != mTaskPool.end(); it++)
		{
	    	if ((*it)->IO()->TaskType() != TASK_UNIX)
	    	{
	    		(*it)->DeleteIO();
	    	}
			SAFE_DELETE(*it);
		}
	}
	mTaskPool.clear();
	mTaskCount = 0;
}

template <typename T>
int wServer<T>::RemoveEpoll(wTask* pTask)
{
	int iFD = pTask->IO()->FD();
	mEpollEvent.data.fd = iFD;
	if(epoll_ctl(mEpollFD, EPOLL_CTL_DEL, iFD, &mEpollEvent) < 0)
	{
		mErr = errno;
		LOG_ERROR(ELOG_KEY, "[system] epoll remove socket fd(%d) error : %s", iFD, strerror(mErr));
		return -1;
	}
	return 0;
}

//返回下一个迭代器
template <typename T>
std::vector<wTask*>::iterator wServer<T>::RemoveTaskPool(wTask* pTask)
{
    std::vector<wTask*>::iterator it = std::find(mTaskPool.begin(), mTaskPool.end(), pTask);
    if (it != mTaskPool.end())
    {
    	if ((*it)->IO()->TaskType() != TASK_UNIX)
    	{
    		(*it)->DeleteIO();
    	}
    	SAFE_DELETE(*it);
        it = mTaskPool.erase(it);
    }
    mTaskCount = mTaskPool.size();
    return it;
}

template <typename T>
void wServer<T>::Run()
{
	//...
}

template <typename T>
void wServer<T>::CheckTimer()
{
	unsigned long long iInterval = (unsigned long long)(GetTickCount() - mLastTicker);
	if(iInterval < 100) 	//100ms
	{
		return;
	}

	mLastTicker += iInterval;
	if(mCheckTimer.CheckTimer(iInterval))
	{
		CheckTimeout();
	}
}

template <typename T>
void wServer<T>::CheckTimeout()
{
	if (!mIsCheckTimer)
	{
		return;
	}

	unsigned long long iNowTime = GetTickCount();
	unsigned long long iIntervalTime;
	if(mTaskPool.size() > 0)
	{
		SOCK_TYPE sockType;
		TASK_TYPE taskType;
		vector<wTask*>::iterator iter;
		for(iter = mTaskPool.begin(); iter != mTaskPool.end(); iter++)
		{
			sockType = (*iter)->IO()->SockType();
			taskType = (*iter)->IO()->TaskType();
			if (taskType == TASK_TCP && sockType == SOCK_CONNECT)
			{
				//心跳检测
				iIntervalTime = iNowTime - (*iter)->IO()->SendTime();	//上一次发送时间间隔
				if (iIntervalTime >= KEEPALIVE_TIME)	//3s
				{
					bool bDelTask = false;
					if (mIsCheckTimer == true)	//使用业务层心跳机制
					{
						(*iter)->Heartbeat();
						if ((*iter)->HeartbeatOutTimes())
						{
							bDelTask = true;
						}
					}
					else	//使用keepalive保活机制（此逻辑一般不会被激活。也不必在业务层发送心跳，否则就失去了keepalive原始意义）
					{
						if ((*iter)->Heartbeat() > 0)
						{
							(*iter)->ClearbeatOutTimes();
						}
						if ((*iter)->HeartbeatOutTimes())
						{
							bDelTask = true;
						}
					}

					//关闭无用连接
					if (bDelTask)
					{
						LOG_ERROR(ELOG_KEY, "[system] client fd(%d) heartbeat pass limit times, close it", (*iter)->IO()->FD());
						if (RemoveEpoll(*iter) >= 0)
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
