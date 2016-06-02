
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "wUDSocket.h"

wUDSocket::wUDSocket()
{
	Initialize();
}

wUDSocket::~wUDSocket() 
{
	//...
}

void wUDSocket::Initialize()
{
	mIOType = TYPE_SOCK;
	mIOFlag = FLAG_RVSD;
	mTaskType = TASK_UNIXD;
}

int wUDSocket::Open()
{
	if ((mFD = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
	{
		mErr = errno;
		return -1;
	}
	return mFD;
}

int wUDSocket::Bind(string sPathAddr)
{
	if (mFD == FD_UNKNOWN)
	{
		return -1;
	}

	struct sockaddr_un stSocketAddr;
	memset(&stSocketAddr, 0, sizeof(stSocketAddr));
	stSocketAddr.sun_family = AF_UNIX;
	strncpy(stSocketAddr.sun_path, sPathAddr.c_str(), sizeof(stSocketAddr.sun_path) - 1);

	if (bind(mFD, (struct sockaddr *)&stSocketAddr, sizeof(stSocketAddr)) < 0)
	{
		mErr = errno;
		Close();
		return -1;
	}
	return 0;
}

int wUDSocket::Listen(string sPathAddr)
{
	if (mFD == FD_UNKNOWN)
	{
		return -1;
	}
	mHost = sPathAddr;
	mSockType = SOCK_LISTEN;
	mIOFlag = FLAG_RECV;

	if (Bind(sPathAddr) < 0)
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

int wUDSocket::Connect(string sPathAddr, float fTimeout)
{
	if (mFD == FD_UNKNOWN)
	{
		return -1;
	}
	mHost = sPathAddr;
	mSockType = SOCK_CONNECT;
	mIOFlag = FLAG_RECV;

	string sTmpSock = "tmpsock";
	sTmpSock += getpid();
	if (Bind(sTmpSock) < 0)
	{
		mErr = errno;
		Close();
		return -1;
	}

	struct sockaddr_un stSockAddr;
	memset(&stSockAddr, 0, sizeof(stSockAddr));
	stSockAddr.sun_family = AF_UNIX;
	strncpy(stSockAddr.sun_path, mHost.c_str(), sizeof(stSockAddr.sun_path) - 1);

	//超时设置
	if (fTimeout > 0)
	{
		SetNonBlock();
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
                        return -1;
                    }
                    if(iVal > 0)
                    {
                    	mErr = errno;
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
int wUDSocket::Accept(struct sockaddr* pClientSockAddr, socklen_t *pSockAddrSize)
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

	return iNewFD;
}

/**
 *  从客户端接收原始数据
 *  return ：<0 对端发生错误|消息超长|对端关闭(FIN_WAIT) =0 稍后重试 >0 接受字符
 */
ssize_t wUDSocket::RecvBytes(char *vArray, size_t vLen)
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
			
			return iRecvLen;
		}
	}
}

/**
 *  原始发送客户端数据
 *  return ：<0 对端发生错误 >=0 发送字符
 */
ssize_t wUDSocket::SendBytes(char *vArray, size_t vLen)
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
			
			return iSendLen;
		}
	}
}
