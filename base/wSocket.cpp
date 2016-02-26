
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include <arpa/inet.h>

#include "wLog.h"
#include "wMisc.h"
#include "wSocket.h"

int wSocket::SetNonBlock(bool bNonblock)
{
	if(mSocketFD < 0) 
	{
		return -1;
	}

	int iFlags = fcntl(mSocketFD, F_GETFL, 0);
	if( iFlags == -1 ) 
	{
		return -1;
	}

	return fcntl(mSocketFD, F_SETFL, (bNonblock == true ? iFlags | O_NONBLOCK : iFlags & ~O_NONBLOCK));
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
	if(mSocketFD < 0) 
	{
		return -1;
	}

	struct timeval stSendTimeval;
	stSendTimeval.tv_sec = iTimeout<0 ? 0 : iTimeout;
	stSendTimeval.tv_usec = 0;
	
	if(setsockopt(mSocketFD, SOL_SOCKET, SO_SNDTIMEO, &stSendTimeval, sizeof(stSendTimeval)) == -1)  
    {
        return -1;  
    }
    return 0;
}

int wSocket::SetRecvTimeout(int iTimeout)
{
	if(mSocketFD < 0) 
	{
		return -1;
	}

	struct timeval stRecvTimeval;
	stRecvTimeval.tv_sec = iTimeout<0 ? 0 : iTimeout;
	stRecvTimeval.tv_usec = 0;
	
	if(setsockopt(mSocketFD, SOL_SOCKET, SO_RCVTIMEO, &stRecvTimeval, sizeof(stRecvTimeval)) == -1)  
    {
        return -1;  
    }
    return 0;
}

/**
 *  从客户端接收原始数据
 *  return ：<0 对端发生错误|消息超长 =0 对端关闭(FIN_WAIT1)  >0 接受字符
 */
int wSocket::RecvBytes(char *vArray, int vLen)
{
	mRecvTime = GetTickCount();
	int iRecvLen;
	while(1)
	{
		iRecvLen = recv(mSocketFD, vArray, vLen, 0);
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
			return iRecvLen;
		}
	}
}

/**
 *  原始发送客户端数据
 *  return ：<0 对端发生错误 >=0 发送字符
 */
int wSocket::SendBytes(char *vArray, int vLen)
{
	mSendTime = GetTickCount();
	int iSendLen;
	int iLeftLen = vLen;
	int iHaveSendLen = 0;

	while(1)
	{
		iSendLen = send(mSocketFD, vArray + iHaveSendLen, iLeftLen, 0);
		if( iSendLen > 0 )
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
			LOG_ERROR("default", "SendToClient fd(%d) error: %s", mSocketFD, strerror(errno));
			return iSendLen;
		}
	}
}