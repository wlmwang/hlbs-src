
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include "wSocket.h"

wSocket::wSocket()
{
	Initialize();
}

wSocket::~wSocket() {}

void wSocket::Initialize()
{
	mIOType = TYPE_SOCK;
	mIOFlag = FLAG_RVSD;
	mTaskType = TASK_TCP;
}

int wSocket::Open()
{
	mFD = socket(AF_INET, SOCK_STREAM, 0); 
	if(mFD < 0)
	{
		mErr = errno;
		return -1;
	}
	int iFlags = 1;
	struct linger stLing = {0,0};
	setsockopt(mFD, SOL_SOCKET, SO_REUSEADDR, &iFlags, sizeof(iFlags));
	setsockopt(mFD, SOL_SOCKET, SO_KEEPALIVE, &iFlags, sizeof(iFlags));
	setsockopt(mFD, SOL_SOCKET, SO_LINGER, &stLing, sizeof(stLing));
	
	return mFD;
}

int wSocket::Bind(string sIpAddr ,unsigned int nPort)
{
	if (mFD == FD_UNKNOWN)
	{
		return -1;
	}
	mHost = sIpAddr;
	mPort = nPort;

	struct sockaddr_in stSocketAddr;
	stSocketAddr.sin_family = AF_INET;
	stSocketAddr.sin_port = htons((short)nPort);
	stSocketAddr.sin_addr.s_addr = inet_addr(sIpAddr.c_str());

	if(bind(mFD, (struct sockaddr *)&stSocketAddr, sizeof(stSocketAddr)) < 0)
	{
		mErr = errno;
		return -1;
	}
	return 0;
}

int wSocket::Listen(string sIpAddr ,unsigned int nPort)
{
	if (mFD == FD_UNKNOWN)
	{
		return -1;
	}
	mSockType = SOCK_LISTEN;
	mIOFlag = FLAG_RECV;

	if(Bind(sIpAddr, nPort) < 0)
	{
		return -1;
	}

	//setsockopt socket : 设置发送缓冲大小4M
	int iOptLen = sizeof(socklen_t);
	int iOptVal = 0x400000;
	if(setsockopt(mFD, SOL_SOCKET, SO_SNDBUF, (const void *)&iOptVal, iOptLen) < -1)
	{
		mErr = errno;
		return -1;
	}
	
	if(listen(mFD, LISTEN_BACKLOG) < 0)
	{
		mErr = errno;
		return -1;
	}
	return 0;
}

int wSocket::Connect(string sIpAddr ,unsigned int nPort)
{
	if (mFD == FD_UNKNOWN)
	{
		return -1;
	}

	mHost = sIpAddr;
	mPort = nPort;
	
	mSockType = SOCK_CONNECT;
	mIOFlag = FLAG_RECV;

	sockaddr_in stSockAddr;
	memset(&stSockAddr, 0, sizeof(sockaddr_in));
	stSockAddr.sin_family = AF_INET;
	stSockAddr.sin_port = htons((short)nPort);
	stSockAddr.sin_addr.s_addr = inet_addr(sIpAddr.c_str());;

	socklen_t iOptVal = 100*1024;
	socklen_t iOptLen = sizeof(socklen_t);
	if(setsockopt(mFD, SOL_SOCKET, SO_SNDBUF, (const void *)&iOptVal, iOptLen) != 0)
	{
		mErr = errno;
		return -1;
	}
	if(getsockopt(mFD, SOL_SOCKET, SO_SNDBUF, (void *)&iOptVal, &iOptLen) == 0)
	{
		//
	}
	
	if(connect(mFD, (const struct sockaddr *)&stSockAddr, sizeof(stSockAddr)) < 0)
	{
		mErr = errno;
		return -1;
	}
	return 0;
}

int wSocket::Accept(struct sockaddr* pClientSockAddr, socklen_t *pSockAddrSize)
{
	if (mFD == FD_UNKNOWN || mSockType != SOCK_LISTEN)
	{
		return -1;
	}
	
	int iNewFD = accept(mFD, pClientSockAddr, pSockAddrSize);
	if(iNewFD < 0)
	{
		mErr = errno;
	    return -1;
    }

	//setsockopt socket：设置发送缓冲大小3M
	int iOptLen = sizeof(socklen_t);
	int iOptVal = 0x300000;
	if(setsockopt(iNewFD, SOL_SOCKET, SO_SNDBUF, (const void *)&iOptVal, iOptLen) < -1)
	{
		mErr = errno;
		return -1;
	}
	return iNewFD;
}

int wSocket::SetTimeout(int iTimeout)
{
	if(SetSendTimeout(iTimeout) < 0)
	{
		return -1;
	}
	if(SetRecvTimeout(iTimeout) < 0)
	{
		return -1;
	}
	return 0;
}

int wSocket::SetSendTimeout(int iTimeout)
{
	if(mFD == FD_UNKNOWN || mIOType != TYPE_SOCK) 
	{
		return -1;
	}

	struct timeval stSendTimeval;
	stSendTimeval.tv_sec = iTimeout<0 ? 0 : iTimeout;
	stSendTimeval.tv_usec = 0;
	if(setsockopt(mFD, SOL_SOCKET, SO_SNDTIMEO, &stSendTimeval, sizeof(stSendTimeval)) == -1)  
    {
        return -1;  
    }
    return 0;
}

int wSocket::SetRecvTimeout(int iTimeout)
{
	if(mFD == FD_UNKNOWN || mIOType != TYPE_SOCK) 
	{
		return -1;
	}

	struct timeval stRecvTimeval;
	stRecvTimeval.tv_sec = iTimeout<0 ? 0 : iTimeout;
	stRecvTimeval.tv_usec = 0;
	if(setsockopt(mFD, SOL_SOCKET, SO_RCVTIMEO, &stRecvTimeval, sizeof(stRecvTimeval)) == -1)  
    {
        return -1;  
    }
    return 0;
}

/**
 *  从客户端接收原始数据
 *  return ：<0 对端发生错误|消息超长 =0 对端关闭(FIN_WAIT1) >0 接受字符
 */
ssize_t wSocket::RecvBytes(char *vArray, size_t vLen)
{
	mRecvTime = GetTickCount();
	
	ssize_t iRecvLen;
	while(true)
	{
		iRecvLen = recv(mFD, vArray, vLen, 0);
		if(iRecvLen > 0)
		{
			return iRecvLen;
		}
		else
		{
			mErr = errno;
			if(iRecvLen < 0 && mErr == EINTR)	//中断
			{
				continue;
			}
			if(iRecvLen < 0 && (mErr == EAGAIN || mErr == EWOULDBLOCK))	//缓冲区满|超时
			{
				//continue;
			}
			
			return iRecvLen;
		}
	}
}

/**
 *  原始发送客户端数据
 *  return ：<0 对端发生错误 >=0 发送字符
 */
ssize_t wSocket::SendBytes(char *vArray, size_t vLen)
{
	mSendTime = GetTickCount();
	
	ssize_t iSendLen;
	size_t iLeftLen = vLen;
	size_t iHaveSendLen = 0;
	while(true)
	{
		iSendLen = send(mFD, vArray + iHaveSendLen, iLeftLen, 0);
		if(iSendLen > 0)
		{
			iLeftLen -= iSendLen;
			iHaveSendLen += iSendLen;
			if(iLeftLen == 0)
			{
				return vLen;
			}
		}
		else
		{
			mErr = errno;
			if(mErr == EINTR) //中断
			{
				continue;
			}
			if(iSendLen < 0 && (mErr == EAGAIN || mErr == EWOULDBLOCK))	//缓冲区满|超时
			{
				//continue;
			}
			
			return iSendLen;
		}
	}
}
