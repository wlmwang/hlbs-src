
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
	setsockopt(mFD, SOL_SOCKET, SO_LINGER, &stLing, sizeof(stLing));	//优雅断开
	
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
		Close();
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
		mErr = errno;
		Close();
		return -1;
	}

	//setsockopt socket : 设置发送缓冲大小4M
	int iOptLen = sizeof(socklen_t);
	int iOptVal = 0x400000;
	if(setsockopt(mFD, SOL_SOCKET, SO_SNDBUF, (const void *)&iOptVal, iOptLen) < -1)
	{
		mErr = errno;
		Close();
		return -1;
	}
	
	if(listen(mFD, LISTEN_BACKLOG) < 0)
	{
		mErr = errno;
		Close();
		return -1;
	}
	return 0;
}

int wSocket::Connect(string sIpAddr ,unsigned int nPort, float fTimeout)
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
		Close();
		return -1;
	}
	if(getsockopt(mFD, SOL_SOCKET, SO_SNDBUF, (void *)&iOptVal, &iOptLen) == 0)
	{
		//log...
	}

	if (fTimeout > 0)
	{
		if(SetNonBlock() < 0)
		{
			SetSendTimeout(fTimeout);	//linux平台下可用
		}
	}

	int iRet = connect(mFD, (const struct sockaddr *)&stSockAddr, sizeof(stSockAddr));
	int iLen , iVal;
	if(fTimeout > 0 && iRet < 0)
	{
		mErr = errno;
		if (mErr == EINPROGRESS)	//连接建立，建立启动但是尚未完成
		{
			struct pollfd stFD;
			int iTimeout = fTimeout * 1000000;

			while (true)
			{
				stFD.fd = mFD;
                stFD.events = POLLIN | POLLOUT;
                iRet = poll(&stFD, 1, iTimeout);

                if(iRet == -1)
                {
                	mErr = errno;
                    if(mErr == EINTR)
                    {
                        continue;
                    }
                    Close();
                    return -1;
                }
                else if(iRet == 0)
                {
                	//tcp connect timeout millisecond=%d
                    Close();
                    return ERR_TIMEO;
                }
                else
                {
                    iLen = sizeof(iVal);
                    iRet = getsockopt(mFD, SOL_SOCKET, SO_ERROR, (char*)&iVal, (socklen_t*)&iLen);
                    if(iRet == -1)
                    {
                    	//ip=%s:%u, tcp connect getsockopt errno=%d,%s
                        Close();
                        mErr = errno;
                        return -1;
                    }
                    if(iVal > 0)
                    {
                    	//ip=%s:%u, tcp connect fail errno=%d,%s
                        Close();
                        mErr = errno;
                        return -1;
                    }
                    break;	//连接成功
                }
			}
		}
		else
		{
			//ip=%s:%u, tcp connect directly errno=%d,%s
			Close();
			return -1;
		}
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

int wSocket::SetTimeout(float fTimeout)
{
	if(SetSendTimeout(fTimeout) < 0)
	{
		return -1;
	}
	if(SetRecvTimeout(fTimeout) < 0)
	{
		return -1;
	}
	return 0;
}

int wSocket::SetSendTimeout(float fTimeout)
{
	if(mFD == FD_UNKNOWN || mIOType != TYPE_SOCK) 
	{
		return -1;
	}

	struct timeval stTimetv;
	stTimetv.tv_sec = (int)fTimeout>=0 ? (int)fTimeout : 0;
	stTimetv.tv_usec = (int)((fTimeout - (int)fTimeout) * 1000000);
	if(stTimetv.tv_usec < 0 || stTimetv.tv_usec >= 1000000 || (stTimetv.tv_sec == 0 && stTimetv.tv_usec == 0))
	{
		stTimetv.tv_sec = 30;
		stTimetv.tv_usec = 0;
	}

	if(setsockopt(mFD, SOL_SOCKET, SO_SNDTIMEO, &stTimetv, sizeof(stTimetv)) == -1)  
    {
        return -1;  
    }
    return 0;
}

int wSocket::SetRecvTimeout(float fTimeout)
{
	if(mFD == FD_UNKNOWN || mIOType != TYPE_SOCK) 
	{
		return -1;
	}

	struct timeval stTimetv;
	stTimetv.tv_sec = (int)fTimeout>=0 ? (int)fTimeout : 0;
	stTimetv.tv_usec = (int)((fTimeout - (int)fTimeout) * 1000000);
	if(stTimetv.tv_usec < 0 || stTimetv.tv_usec >= 1000000 || (stTimetv.tv_sec == 0 && stTimetv.tv_usec == 0))
	{
		stTimetv.tv_sec = 30;
		stTimetv.tv_usec = 0;
	}
	
	if(setsockopt(mFD, SOL_SOCKET, SO_RCVTIMEO, &stTimetv, sizeof(stTimetv)) == -1)  
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
				//usleep(100);
				//continue;
			}
			
			LOG_ERROR(ELOG_KEY, "[runtime] recv fd(%d) error: %s", mFD, strerror(mErr));
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
				//usleep(100);
				//continue;
			}
			
			LOG_ERROR(ELOG_KEY, "send fd(%d) error: %s", mFD, strerror(mErr));
			return iSendLen;
		}
	}
}
