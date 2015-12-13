
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <arpa/inet.h>

#include "wLog.h"
#include "wSocket.h"

int wSocket::SetNonBlock()
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
	return fcntl(mSocketFD, F_SETFL, iFlags|O_NONBLOCK);
}

/**
 *  从客户端接收原始数据
 *  return ：<0 对端发生错误|消息超长 =0 对端关闭(FIN_WAIT1)  >0 接受字符
 */
int wSocket::RecvBytes(char *vArray, int vLen)
{
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
			if(iRecvLen < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))	//缓冲区满
			{
				//waitread
				//continue;
				//在外层做可读事件准备(tcptask)
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
			if(iRecvLen < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))	//缓冲区满
			{
				//waitwrite
				//continue;
				//在外层做可写事件准备(tcptask)
			}
			LOG_ERROR("default", "SendToClient fd(%d) error: %s", mSocketFD, strerror(errno));
			return iSendLen;
		}
	}
}