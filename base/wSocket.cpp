
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
	mIPAddr = "";
	mPort = 0;
	mIOType = TYPE_SOCK;
	mSockType = SOCK_UNKNOWN;
	mSockStatus = STATUS_UNKNOWN;
}

int wSocket::Open()
{
	int iSocketFD = socket(AF_INET, SOCK_STREAM, 0); 
	if(iSocketFD < 0)
	{
		LOG_ERROR(ELOG_KEY, "[startup] Create socket failed: %s", strerror(errno));
		return -1;
	}
	mFD = iSocketFD;

	int iFlags = 1;
	struct linger stLing = {0,0};
	setsockopt(mFD, SOL_SOCKET, SO_REUSEADDR, &iFlags, sizeof(iFlags));
	setsockopt(mFD, SOL_SOCKET, SO_KEEPALIVE, &iFlags, sizeof(iFlags));
	setsockopt(mFD, SOL_SOCKET, SO_LINGER, &stLing, sizeof(stLing));
	
	return mFD;
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
			if(iRecvLen < 0 && errno == EINTR)	//中断
			{
				continue;
			}
			if(iRecvLen < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))	//缓冲区满|超时
			{
				//可读事件准备(tcptask)
				//waitread
				usleep(100);
				continue;
			}
			
			LOG_ERROR(ELOG_KEY, "recv fd(%d) error: %s", mFD, strerror(errno));
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
			if(errno == EINTR) //中断
			{
				continue;
			}
			if(iSendLen < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))	//缓冲区满|超时
			{
				//可写事件准备(tcptask)
				//waitwrite
				usleep(100);
				continue;
			}
			
			LOG_ERROR(ELOG_KEY, "send fd(%d) error: %s", mFD, strerror(errno));
			return iSendLen;
		}
	}
}
