
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_TCP_CLIENT_H_
#define _W_TCP_CLIENT_H_

//#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <typeinfo>
#include <string>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include "wType.h"
#include "wTimer.h"
#include "wMisc.h"
#include "wLog.h"
#include "wSocket.h"
#include "wTcpTask.h"

enum CLIENT_STATUS
{
	CLIENT_STATUS_INIT,
	CLIENT_STATUS_QUIT,	
	CLIENT_STATUS_RUNNING
};

template <typename T>
class wTcpClient
{
	public:
		virtual ~wTcpClient();
		
        wTcpClient(int iType, string sClientName, int vInitFlag = true);
	    	
		wTcpClient(const wTcpClient&);
		
		void Initialize();
		
		int ListeningRecv();
		int ListeningSend();
		
		int ConnectToServer(const char *vIPAddress, unsigned short vPort);
		
		int ReConnectToServer();

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
		
		string GetClientName()
		{
			return mClientName;
		}
		
		int GetType()
		{
			return mType;
		}
		
		void Final();
		
		virtual wTcpTask* NewTcpTask(wSocket *pSocket);

		virtual void PrepareStart();
		virtual void Start();
		
		void CheckTimer();
		void CheckReconnect();

		CLIENT_STATUS mStatus;
		
		wTcpTask* TcpTask() { return mTcpTask; }
		
	protected:
		int mType;
		string mClientName;		
		wTcpTask* mTcpTask;

		unsigned long long mLastTicker;
		wTimer mReconnectTimer;
		int mReconnectTimes;
};

template <typename T>
wTcpClient<T>::wTcpClient(int iType, string sClientName, int vInitFlag)
{
	if(vInitFlag)
	{
		mStatus = CLIENT_STATUS_INIT;
		Initialize();
	}
	mType = iType;
	mClientName = sClientName;
}

template <typename T>
void wTcpClient<T>::Initialize()
{
	mLastTicker = GetTickCount();
	mReconnectTimer = wTimer(KEEPALIVE_TIME);
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
	SetStatus();
	SAFE_DELETE(mTcpTask);
}

template <typename T>
int wTcpClient<T>::ReConnectToServer()
{
	if (mTcpTask != NULL && mTcpTask->Socket() != NULL)
	{
		if (mTcpTask->Socket()->SocketFD() < 0)
		{
			return ConnectToServer(mTcpTask->Socket()->IPAddr().c_str(), mTcpTask->Socket()->Port());
		}
		return 0;
	}
	return -1;
}

template <typename T>
int wTcpClient<T>::ConnectToServer(const char *vIPAddress, unsigned short vPort)
{
	if(vIPAddress == NULL) 
	{
		return -1;
	}

	if(mTcpTask != NULL)
	{
		SAFE_DELETE(mTcpTask);
	}
	
	int iSocketFD = socket(AF_INET, SOCK_STREAM, 0);
	if( iSocketFD < 0 )
	{
		LOG_ERROR("default", "create socket failed: %s", strerror(errno));
		return -1;
	}

	sockaddr_in stSockAddr;
	memset(&stSockAddr, 0, sizeof(sockaddr_in));
	stSockAddr.sin_family = AF_INET;
	stSockAddr.sin_port = htons(vPort);
	stSockAddr.sin_addr.s_addr = inet_addr(vIPAddress);;

	socklen_t iOptVal = 100*1024;
	socklen_t iOptLen = sizeof(int);

	if(setsockopt(iSocketFD, SOL_SOCKET, SO_SNDBUF, (const void *)&iOptVal, iOptLen) != 0)
	{
		LOG_ERROR("default", "set send buffer size to %d failed: %s", iOptVal, strerror(errno));
		return -1;
	}

	if(getsockopt(iSocketFD, SOL_SOCKET, SO_SNDBUF, (void *)&iOptVal, &iOptLen) == 0)
	{
		LOG_INFO("default", "set send buffer of socket is %d", iOptVal);
	}
	
	int iRet = connect(iSocketFD, (const struct sockaddr *)&stSockAddr, sizeof(stSockAddr));
	if( iRet < 0 )
	{
		LOG_ERROR("default", "connect to server port(%d) failed: %s", vPort, strerror(errno));
		return -1;
	}

	LOG_INFO("default", "connect to %s:%d successfully", inet_ntoa(stSockAddr.sin_addr), vPort);
	
	wSocket* pSocket = new wSocket();
	pSocket->SocketFD() = iSocketFD;
	pSocket->IPAddr() = vIPAddress;
	pSocket->Port() = vPort;
	pSocket->SocketType() = CONNECT_SOCKET;
	pSocket->SocketFlag() = RECV_DATA;
	
	mTcpTask = NewTcpTask(pSocket);
	if(NULL != mTcpTask)
	{
		if(mTcpTask->Socket()->SetNonBlock() < 0) 
		{
			LOG_ERROR("default", "set non block failed: %d, close it", iSocketFD);
			return -1;
		}

		return 0;
	}
	return -1;
}

template <typename T>
void wTcpClient<T>::PrepareStart()
{
	SetStatus(CLIENT_STATUS_RUNNING);
}

template <typename T>
void wTcpClient<T>::Start()
{
	if(IsRunning())
	{
		ListeningRecv();

		CheckTimer();
	}
}

template <typename T>
void wTcpClient<T>::CheckTimer()
{
	int iInterval = (int)(GetTickCount() - mLastTicker);

	if(iInterval < 100) 	//100ms
	{
		return;
	}

	mLastTicker += iInterval;
	
	if(mReconnectTimer.CheckTimer(iInterval))
	{
		CheckReconnect();
	}
}

template <typename T>
void wTcpClient<T>::CheckReconnect()
{
	int iNowTime = time(NULL);
	int iIntervalTime;

	if (mTcpTask == NULL || mTcpTask->Socket()->SocketType() != CONNECT_SOCKET)
	{
		return;
	}
	iIntervalTime = iNowTime - mTcpTask->Socket()->Stamp();
	if (iIntervalTime >= KEEPALIVE_TIME)
	{
		if(mTcpTask->Heartbeat() < 0 && mTcpTask->HeartbeatOutTimes())
		{
			SetStatus();
			LOG_INFO("default", "disconnect server : out of heartbeat times");
		}
		else
		{
			ReConnectToServer();
		}
	}
}

template <typename T>
int wTcpClient<T>::ListeningRecv()
{
	if (mTcpTask != NULL && IsRunning())
	{
		return mTcpTask->ListeningRecv();
	}
	return -1;
}

template <typename T>
int wTcpClient<T>::ListeningSend()
{
	if (mTcpTask != NULL && IsRunning())
	{
		return mTcpTask->ListeningSend();
	}
	return -1;
}

template <typename T>
wTcpTask* wTcpClient<T>::NewTcpTask(wSocket *pSocket)
{
	wTcpTask *pTcpTask = NULL;
	try
	{
		T* pT = new T(pSocket);
		pTcpTask = dynamic_cast<wTcpTask *>(pT);
	}
	catch(std::bad_cast& vBc)
	{
		LOG_ERROR("default", "bad_cast caught: : %s", vBc.what());
	}
	return pTcpTask;
}

#endif
