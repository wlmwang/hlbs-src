
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

template <typename T>
wTcpClient<T>::wTcpClient(int iType, string sClientName)
{
	Initialize();
	mType = iType;
	mClientName = sClientName;
}

template <typename T>
void wTcpClient<T>::Initialize()
{
	mStatus = CLIENT_INIT;
	mLastTicker = GetTickCount();
	mCheckTimer = wTimer(KEEPALIVE_TIME);
	mReconnectTimer = wTimer(KEEPALIVE_TIME);
	mIsCheckTimer = false;
	mReconnectTimes = 0;
	mClientName = "";
	mType = 0;
	mTcpTask = NULL;
}

template <typename T>
wTcpClient<T>::~wTcpClient()
{
	Final();
}

template <typename T>
void wTcpClient<T>::Final()
{
	SAFE_DELETE(mTcpTask);
}

template <typename T>
int wTcpClient<T>::ReConnectToServer()
{
	if (mTcpTask != NULL && mTcpTask->IO() != NULL)
	{
		if (mTcpTask->IO()->FD() < 0)
		{
			return ConnectToServer(mTcpTask->IO()->Host().c_str(), mTcpTask->IO()->Port());
		}
		return 0;
	}
	return -1;
}

template <typename T>
int wTcpClient<T>::ConnectToServer(const char *vIPAddress, unsigned short vPort)
{
	if (vIPAddress == NULL || vPort == 0) 
	{
		return -1;
	}
	SAFE_DELETE(mTcpTask);
	
	wSocket* pSocket = new wSocket();
	int iSocketFD = pSocket->Open();
	if (iSocketFD < 0)
	{
		LOG_ERROR(ELOG_KEY, "[runtime] create socket failed: %s", strerror(pSocket->Errno()));
		return -1;
	}

	int iRet = pSocket->Connect(vIPAddress, vPort);
	if (iRet < 0)
	{
		LOG_ERROR(ELOG_KEY, "[runtime] connect to server port(%d) failed: %s", vPort, strerror(pSocket->Errno()));
		return -1;
	}
	LOG_DEBUG(ELOG_KEY, "[runtime] connect to %s:%d successfully", vIPAddress, vPort);
	
	mTcpTask = NewTcpTask(pSocket);
	if (NULL != mTcpTask)
	{
		if (mTcpTask->Verify() < 0 || mTcpTask->VerifyConn() < 0)
		{
			LOG_ERROR(ELOG_KEY, "[runtime] connect illegal or verify timeout: %d, close it", iSocketFD);
			SAFE_DELETE(mTcpTask);
			return -1;
		}
		mTcpTask->Status() = TASK_RUNNING;
		if (mTcpTask->IO()->SetNonBlock() < 0) 
		{
			LOG_ERROR(ELOG_KEY, "[runtime] set non block failed: %d, close it", iSocketFD);
			SAFE_DELETE(mTcpTask);
			return -1;
		}
		return 0;
	}
	return -1;
}

template <typename T>
void wTcpClient<T>::PrepareStart()
{
	mStatus = CLIENT_RUNNING;
}

template <typename T>
void wTcpClient<T>::Start(bool bDaemon)
{
	//...
}

template <typename T>
void wTcpClient<T>::CheckTimer()
{
	unsigned long long iInterval = (unsigned long long)(GetTickCount() - mLastTicker);
	if (iInterval < 100) 	//100ms
	{
		return;
	}

	mLastTicker += iInterval;
	if(mCheckTimer.CheckTimer(iInterval))
	{
		CheckTimeout();
	}
	if (mReconnectTimer.CheckTimer(iInterval))
	{
		CheckReconnect();
	}
}

template <typename T>
void wTcpClient<T>::CheckTimeout()
{
	if (!mIsCheckTimer)
	{
		return;
	}

	unsigned long long iNowTime = GetTickCount();
	unsigned long long iIntervalTime;
	if (mTcpTask == NULL)
	{
		return;
	}
	SOCK_TYPE sockType = mTcpTask->IO()->SockType();
	if (sockType != SOCK_CONNECT)
	{
		return;
	}
	
	//心跳检测
	iIntervalTime = iNowTime - mTcpTask->IO()->SendTime();	//上一次发送时间间隔
	if (iIntervalTime >= KEEPALIVE_TIME)
	{
		if (mIsCheckTimer == true)	//使用业务层心跳机制
		{
			mTcpTask->Heartbeat();
			if (mTcpTask->HeartbeatOutTimes())
			{
				//mStatus = CLIENT_QUIT;
				mTcpTask->IO()->FD() = FD_UNKNOWN;
				LOG_DEBUG(ELOG_KEY, "[runtime] disconnect server : out of heartbeat times");
			}
		}
		else	//使用keepalive保活机制（此逻辑一般不会被激活。也不必在业务层发送心跳，否则就失去了keepalive原始意义）
		{
			if (mTcpTask->Heartbeat() > 0)
			{
				mTcpTask->ClearbeatOutTimes();
			}
			if (mTcpTask->HeartbeatOutTimes())
			{
				//mStatus = CLIENT_QUIT;
				mTcpTask->IO()->FD() = FD_UNKNOWN;
				LOG_DEBUG(ELOG_KEY, "[runtime] disconnect server : out of keepalive times");
			}
		}
	}
}

template <typename T>
void wTcpClient<T>::CheckReconnect()
{
	unsigned long long iNowTime = GetTickCount();
	unsigned long long iIntervalTime;
	if (mTcpTask == NULL)
	{
		return;
	}
	SOCK_TYPE sockType = mTcpTask->IO()->SockType();
	if (sockType != SOCK_CONNECT)
	{
		return;
	}
	
	iIntervalTime = iNowTime - mTcpTask->IO()->SendTime();	//上一次发送时间间隔
	if (iIntervalTime >= KEEPALIVE_TIME)
	{
		if (mTcpTask->IO() != NULL && mTcpTask->IO()->FD() == FD_UNKNOWN)
		{
			if (ReConnectToServer() == 0)
			{
				LOG_INFO(ELOG_KEY, "[reconnect] success to reconnect : ip(%s) port(%d)", mTcpTask->IO()->Host().c_str(), mTcpTask->IO()->Port());
			}
			else
			{
				LOG_ERROR(ELOG_KEY, "[reconnect] failed to reconnect : ip(%s) port(%d)", mTcpTask->IO()->Host().c_str(), mTcpTask->IO()->Port());
			}
		}
	}
}

template <typename T>
wTcpTask* wTcpClient<T>::NewTcpTask(wIO *pIO)
{
	wTcpTask *pTcpTask = NULL;
	try
	{
		T* pT = new T(pIO);
		pTcpTask = dynamic_cast<wTcpTask *>(pT);
	}
	catch(std::bad_cast& vBc)
	{
		LOG_ERROR(ELOG_KEY, "[runtime] bad_cast caught: : %s", vBc.what());
	}
	return pTcpTask;
}
