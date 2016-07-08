
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "wSocket.h"

wSocket::wSocket()
{
	mIOType = TYPE_SOCK;
	mIOFlag = FLAG_RVSD;
	mTaskType = TASK_TCP;
}

int wSocket::Open()
{
	if ((mFD = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		mErr = errno;
		return -1;
	}

	int iFlags = 1;
	if (setsockopt(mFD, SOL_SOCKET, SO_REUSEADDR, &iFlags, sizeof(iFlags)) == -1)	//端口重用
	{
		mErr = errno;
		Close();
		return -1;
	}

	struct linger stLing = {0, 0};
	if (setsockopt(mFD, SOL_SOCKET, SO_LINGER, &stLing, sizeof(stLing)) == -1)	//优雅断开
	{
		mErr = errno;
		Close();
		return -1;
	}

	if (SetKeepAlive(KEEPALIVE_TIME, KEEPALIVE_TIME, KEEPALIVE_CNT) < 0)	//启用保活机制
	{
		return -1;
	}
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

	if (bind(mFD, (struct sockaddr *)&stSocketAddr, sizeof(stSocketAddr)) < 0)
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

	if (Bind(sIpAddr, nPort) < 0)
	{
		mErr = errno;
		Close();
		return -1;
	}

	//设置发送缓冲大小4M
	int iOptLen = sizeof(socklen_t);
	int iOptVal = 0x400000;
	if (setsockopt(mFD, SOL_SOCKET, SO_SNDBUF, (const void *)&iOptVal, iOptLen) == -1)
	{
		mErr = errno;
		Close();
		return -1;
	}
	
	if (listen(mFD, LISTEN_BACKLOG) < 0)
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
	if (setsockopt(mFD, SOL_SOCKET, SO_SNDBUF, (const void *)&iOptVal, iOptLen) == -1)
	{
		mErr = errno;
		Close();
		return -1;
	}

	//超时设置
	if (fTimeout > 0)
	{
		if (SetNonBlock() < 0)
		{
			SetSendTimeout(fTimeout);	//linux平台下可用
		}
	}

	int iRet = connect(mFD, (const struct sockaddr *)&stSockAddr, sizeof(stSockAddr));
	if (fTimeout > 0 && iRet < 0)
	{
		if (errno == EINPROGRESS)	//建立启动但是尚未完成
		{
			int iLen, iVal, iRet;
			struct pollfd stFD;
			int iTimeout = fTimeout * 1000000;	//微妙
			while (true)
			{
				stFD.fd = mFD;
                stFD.events = POLLIN | POLLOUT;
                iRet = poll(&stFD, 1, iTimeout);
                if (iRet == -1)
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
                    Close();
                    LOG_ERROR(ELOG_KEY, "[system] tcp connect timeout millisecond=%d", iTimeout);
                    return ERR_TIMEO;
                }
                else
                {
                    iLen = sizeof(iVal);
                    iRet = getsockopt(mFD, SOL_SOCKET, SO_ERROR, (char*)&iVal, (socklen_t*)&iLen);
                    if(iRet == -1)
                    {
                    	mErr = errno;
                        Close();
                        LOG_ERROR(ELOG_KEY, "[system] ip=%s:%d, tcp connect getsockopt errno=%d,%s", mHost.c_str(), mPort, mErr, strerror(mErr));
                        return -1;
                    }
                    if(iVal > 0)
                    {
                    	mErr = errno;
                        LOG_ERROR(ELOG_KEY, "[system] ip=%s:%d, tcp connect fail errno=%d,%s", mHost.c_str(), mPort, mErr, strerror(mErr));
                        Close();
                        return -1;
                    }
                    break;	//连接成功
                }
			}
		}
		else
		{
			mErr = errno;
            LOG_ERROR(ELOG_KEY, "[system] ip=%s:%d, tcp connect directly errno=%d,%s", mHost.c_str(), mPort, mErr, strerror(mErr));
			Close();
			return -1;
		}
	}
	return 0;
}

/**
 *  从客户端接收连接
 *  return ：<0 对端发生错误|对端关闭(FIN_WAIT) =0 稍后重试 >0 文件描述符
 */
int wSocket::Accept(struct sockaddr* pClientSockAddr, socklen_t *pSockAddrSize)
{
	if (mFD == FD_UNKNOWN || mSockType != SOCK_LISTEN)
	{
		return -1;
	}

	int iNewFD = 0;
	do {
		iNewFD = accept(mFD, pClientSockAddr, pSockAddrSize);
		if (iNewFD < 0)
		{
			mErr = errno;
			if (mErr == EINTR)	//中断
			{
				continue;
			}
			if (mErr == EAGAIN || mErr == EWOULDBLOCK)	//没有新连接
			{
				return 0;
			}
			break;
	    }
	    break;
	} while (true);

	if (iNewFD <= 0)
	{
		return -1;
	}

	//设置发送缓冲大小3M
	int iOptLen = sizeof(socklen_t);
	int iOptVal = 0x300000;
	if (setsockopt(iNewFD, SOL_SOCKET, SO_SNDBUF, (const void *)&iOptVal, iOptLen) == -1)
	{
		mErr = errno;
		return -1;
	}
	return iNewFD;
}

int wSocket::SetTimeout(float fTimeout)
{
	if (SetSendTimeout(fTimeout) < 0)
	{
		return -1;
	}
	if (SetRecvTimeout(fTimeout) < 0)
	{
		return -1;
	}
	return 0;
}

int wSocket::SetSendTimeout(float fTimeout)
{
	if (mFD == FD_UNKNOWN || mIOType != TYPE_SOCK) 
	{
		return -1;
	}

	struct timeval stTimetv;
	stTimetv.tv_sec = (int)fTimeout>=0 ? (int)fTimeout : 0;
	stTimetv.tv_usec = (int)((fTimeout - (int)fTimeout) * 1000000);
	if (stTimetv.tv_usec < 0 || stTimetv.tv_usec >= 1000000 || (stTimetv.tv_sec == 0 && stTimetv.tv_usec == 0))
	{
		stTimetv.tv_sec = 30;
		stTimetv.tv_usec = 0;
	}

	if (setsockopt(mFD, SOL_SOCKET, SO_SNDTIMEO, &stTimetv, sizeof(stTimetv)) == -1)  
    {
    	mErr = errno;
        return -1;  
    }
    return 0;
}

int wSocket::SetRecvTimeout(float fTimeout)
{
	if (mFD == FD_UNKNOWN || mIOType != TYPE_SOCK) 
	{
		return -1;
	}

	struct timeval stTimetv;
	stTimetv.tv_sec = (int)fTimeout>=0 ? (int)fTimeout : 0;
	stTimetv.tv_usec = (int)((fTimeout - (int)fTimeout) * 1000000);
	if (stTimetv.tv_usec < 0 || stTimetv.tv_usec >= 1000000 || (stTimetv.tv_sec == 0 && stTimetv.tv_usec == 0))
	{
		stTimetv.tv_sec = 30;
		stTimetv.tv_usec = 0;
	}
	
	if (setsockopt(mFD, SOL_SOCKET, SO_RCVTIMEO, &stTimetv, sizeof(stTimetv)) == -1)  
    {
    	mErr = errno;
        return -1;  
    }
    return 0;
}

int wSocket::SetKeepAlive(int iIdle, int iIntvl, int iCnt)
{
	if (mFD == FD_UNKNOWN || mIOType != TYPE_SOCK) 
	{
		return -1;
	}

	int iFlags = 1;
	if (setsockopt(mFD, SOL_SOCKET, SO_KEEPALIVE, &iFlags, sizeof(iFlags)) == -1)  
    {
    	mErr = errno;
        return -1;  
    }
	if (setsockopt(mFD, IPPROTO_TCP, TCP_KEEPIDLE, &iIdle, sizeof(iIdle)) == -1)  
    {
    	mErr = errno;
        return -1;  
    }
	if (setsockopt(mFD, IPPROTO_TCP, TCP_KEEPINTVL, &iIntvl, sizeof(iIntvl)) == -1)  
    {
    	mErr = errno;
        return -1;  
    }
	if (setsockopt(mFD, IPPROTO_TCP, TCP_KEEPCNT, &iCnt, sizeof(iCnt)) == -1)  
    {
    	mErr = errno;
        return -1;  
    }

    //Linux Kernel 2.6.37
    //如果发送出去的数据包在十秒内未收到ACK确认，则下一次调用send或者recv，则函数会返回-1，errno设置为ETIMEOUT
#ifdef TCP_USER_TIMEOUT
    unsigned int iTimeout = 10000;
	if (setsockopt(mFD, IPPROTO_TCP, TCP_USER_TIMEOUT, &iTimeout, sizeof(iTimeout)) == -1)  
    {
    	mErr = errno;
        return -1;
    }
#endif
    
    return 0;
}

/**
 *  从客户端接收原始数据
 *  return ：<0 对端发生错误|消息超长|对端关闭(FIN_WAIT) =0 稍后重试 >0 接受字符
 */
ssize_t wSocket::RecvBytes(char *vArray, size_t vLen)
{
	mRecvTime = GetTickCount();
	
	ssize_t iRecvLen;
	while (true)
	{
		iRecvLen = recv(mFD, vArray, vLen, 0);
		if (iRecvLen > 0)
		{
			return iRecvLen;
		}
		else if (iRecvLen == 0)	//关闭
		{
			return ERR_CLOSED;	//FIN
		}
		else
		{
			mErr = errno;
			if (mErr == EINTR)	//中断
			{
				continue;
			}
			if (mErr == EAGAIN || mErr == EWOULDBLOCK)	//暂时无数据可读，可以继续读，或者等待epoll的后续通知
			{
				return 0;
			}
			
			LOG_ERROR(ELOG_KEY, "[system] recv fd(%d) error: %s", mFD, strerror(mErr));
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
	while (true)
	{
		iSendLen = send(mFD, vArray + iHaveSendLen, iLeftLen, 0);
		if (iSendLen >= 0)
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
			if (mErr == EINTR) //中断
			{
				continue;
			}
			if (iSendLen < 0 && (mErr == EAGAIN || mErr == EWOULDBLOCK))	//当前缓冲区写满，可以继续写，或者等待epoll的后续通知
			{
				return 0;
			}
			
			LOG_ERROR(ELOG_KEY, "[system] send fd(%d) error: %s", mFD, strerror(mErr));
			return iSendLen;
		}
	}
}
