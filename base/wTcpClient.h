
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_TCP_CLIENT_H_
#define _W_TCP_CLIENT_H_

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
#include "wMisc.h"
#include "wLog.h"
#include "wSocket.h"
#include "wTcpTask.h"

enum CLIENT_STATUS
{
	CLIENT_STATUS_INIT,				//初始化状态
	CLIENT_STATUS_QUIT,				//进入关闭状态
	CLIENT_STATUS_RUNNING			//正常运行状态模式
};

enum ETCPClientError
{
	ETC_IPADDR_NOT_VALID		= -1,		// IP地址非法
	ETC_SOCKET_CREATE_FAILED	= -2,		// socket创建失败
	ETC_SET_BUFF_SIZE_FAILED	= -3,		// 设置缓冲区大小失败
	ETC_CONNECT_FAILED			= -4,		// 连接失败
	ETC_ARRAY_IS_NULL			= -5,		// 接收数组为null
	ETC_BUFFER_POINTER_ERROR	= -6,		// 缓冲区指针错误
	ETC_MSG_LENGTH_ERROR		= -7,		// 消息长度出错
	ETC_MSG_IS_TOO_LONG			= -8,		// 消息过长
	ETC_SOCKET_NOT_CONNECT		= -9,		// 该socket未连接
	ETC_BUFFER_IS_FULL			= -10,		// 缓冲区已满
	ETC_REMOTE_CLOSE			= -11,		// 远端关闭
	ETC_OTHER_ERROR				= -12,		// 其他错误
	ETC_NO_BUFFER_TO_SAVE		= -13,		// 发送失败，并且没有缓冲区来存储
	ETC_RECV_NO_MSG				= -14,		// 没有接收到消息
};

template <typename T>
class wTcpClient
{
	public:
		void Recv();
		void Send();
		
		int ConnectToServer(char *vIPAddress, unsigned short vPort);
		
		int ReConnectToServer();

		bool IsRunning()
		{
			return CLIENT_STATUS_INIT == mStatus;
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
		
		void Final();
		
		wTcpTask* NewTcpTask(wSocket *pSocket);
		
		virtual ~wTcpClient();
		
        wTcpClient(string sClientName, int vInitFlag = true);
	    	
		//禁用复制函数
		wTcpClient(const wTcpClient&);
		
        void Initialize();

		CLIENT_STATUS mStatus;	//服务器当前状态
		//定时记录器
		unsigned long long mLastTicker;	//服务器当前时间
		
		wTcpTask* TcpTask() { return mTcpTask; }
		wSocket* Socket() { return mSocket; }
	protected:
		string mClientName;
		wSocket* mSocket;	//Connect Socket(主服务socket对象)
		wTcpTask* mTcpTask;
		
		//unsigned int mTimeout;
};

template <typename T>
wTcpClient<T>::wTcpClient(string sClientName, int vInitFlag)
{
	if(vInitFlag)
	{
		mStatus = CLIENT_STATUS_INIT;
		Initialize();
	}
	mClientName = sClientName;
}

template <typename T>
wTcpClient<T>::~wTcpClient()
{
	Final();
}

template <typename T>
void wTcpClient<T>::Initialize()
{
	mLastTicker = GetTickCount();
	mClientName = "";
	mSocket = NULL;
	mTcpTask = NULL;
}

template <typename T>
void wTcpClient<T>::Final()
{
	SAFE_DELETE(mSocket);
	SAFE_DELETE(mTcpTask);
}

template <typename T>
int wTcpClient<T>::ReConnectToServer()
{
	if (mSocket != NULL)
	{
		if (mSocket->SocketFD < 0)
		{
			return ConnectToServer(mSocket->IPAddr().c_str(), mSocket->Port());
		}
		return 0;
	}
	return -1;
}

template <typename T>
int wTcpClient<T>::ConnectToServer(char *vIPAddress, unsigned short vPort)
{
	if(vIPAddress == NULL) 
	{
		return ETC_IPADDR_NOT_VALID;
	}
	//先关闭
	if(mSocket != NULL)
	{
		SAFE_DELETE(mSocket);
	}
	
	int iSocketFD = socket(AF_INET, SOCK_STREAM, 0);
	if( iSocketFD < 0 )
	{
		LOG_ERROR("default", "create socket failed: %s", strerror(errno));
		return ETC_SOCKET_CREATE_FAILED;
	}

	sockaddr_in stSockAddr;
	memset(&stSockAddr, 0, sizeof(sockaddr_in));
	stSockAddr.sin_family = AF_INET;
	stSockAddr.sin_port = htons(vPort);
	stSockAddr.sin_addr.s_addr = inet_addr(vIPAddress);;

	socklen_t iOptVal = 100*1024;
	socklen_t iOptLen = sizeof(int);
	//设置发送缓冲区大小
	if(setsockopt(iSocketFD, SOL_SOCKET, SO_SNDBUF, (const void *)&iOptVal, iOptLen) != 0)
	{
		LOG_ERROR("default", "set send buffer size to %d failed: %s", iOptVal, strerror(errno));
		return ETC_SET_BUFF_SIZE_FAILED;
	}
	//获取设置完的值
	if(getsockopt(iSocketFD, SOL_SOCKET, SO_SNDBUF, (void *)&iOptVal, &iOptLen) == 0)
	{
		LOG_INFO("default", "set send buffer of socket is %d", iOptVal);
	}
	
	//连接服务端
	int iRet = connect(iSocketFD, (const struct sockaddr *)&stSockAddr, sizeof(stSockAddr));
	if( iRet < 0 )
	{
		LOG_ERROR("default", "connect to server port(%d) failed: %s", vPort, strerror(errno));
		return ETC_CONNECT_FAILED;
	}

	LOG_INFO("default", "connect to %s:%d successfully", inet_ntoa(stSockAddr.sin_addr), vPort);
	
	mSocket = new wSocket();
	mSocket->SocketFD() = iSocketFD;
	mSocket->IPAddr() = vIPAddress;
	mSocket->Port() = vPort;
	mSocket->SocketType() = CONNECT_SOCKET;
	mSocket->SocketFlag() = RECV_DATA;
	
	mTcpTask = NewTcpTask(mSocket);
	if(NULL != mTcpTask)
	{
		if(mTcpTask->Socket()->SetNonBlock() < 0) 
		{
			LOG_ERROR("default", "set non block failed: %d, close it", iSocketFD);
			return -1;
		}
	}
	return 0;
}

template <typename T>
void wTcpClient<T>::Recv()
{
	if(mTcpTask != NULL)
	{
		mTcpTask->ListeningRecv();
	}
}

template <typename T>
void wTcpClient<T>::Send()
{
	//...
}

template <typename T>
wTcpTask* wTcpClient<T>::NewTcpTask(wSocket *pSocket)
{
	wTcpTask *pTcpTask = NULL;
	if(typeid(T) == typeid(wTcpTask))
	{
		T* pT = new T(pSocket);
		pTcpTask = dynamic_cast<wTcpTask *>(pT);
	}
	return pTcpTask;
}

#endif
