
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include <unistd.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>

#include "wLog.h"
#include "wTcpTask.h"

wTcpTask::wTcpTask(int iNewSocket, struct sockaddr_in *stSockAddr)
{
	mSocketFD = iNewSocket;
	mIPAddr = inet_ntoa(stSockAddr->sin_addr);
	mPort = ntohs(stSockAddr->sin_port);
	
	mSocketType = CONNECT_SOCKET;
	mSocketFlag = RECV_DATA;
	mCreateTime = time(NULL);
	
	Initialize();
}

wTcpTask::~wTcpTask()
{
	//...
}

void wTcpTask::Initialize()
{
	memset(&mSendMsgBuff, 0, sizeof(mSendMsgBuff));
	memset(&mRecvMsgBuff, 0, sizeof(mRecvMsgBuff));
}

/**
 * 清除指定TcpTask
 * @param vReason [关闭原因]
 */
void wTcpTask::CloseTcpTask(CLOSE_REASON vReason)
{
	if(vReason == CLOSE_BY_ROUTER || vReason == CLOSE_BY_PEER) 
	{
		//通知各个逻辑服务器这个FD断开连接了
		/*
		memset(mNetHeadPtr, 0, sizeof(*mNetHeadPtr));
		mNetHeadPtr->mClientPort = vInfo.mPort;
		mNetHeadPtr->mSockTime = vInfo.mCreateTime;
		mNetHeadPtr->mTimeStamp = vInfo.mStamp;
		mNetHeadPtr->mClientState = -1;
		*/
		
		/*
		char *pBuffPtr = mSocketBuff + sizeof(int);
		*(unsigned short *)pBuffPtr	= sizeof(*mNetHeadPtr);

		pBuffPtr += sizeof(short);
		mNetHeadPtr->SerializeToArray(pBuffPtr, CLIENT_BUF_LEN - sizeof(int) - sizeof(short));

		*(int *)&mSocketBuff = sizeof(short) + sizeof(*mNetHeadPtr);

		Send2Server((char *)&mSocketBuff, sizeof(int) + sizeof(short) + sizeof(*mNetHeadPtr));
		*/
	}

	if(mSocketFD > 0)
	{
		Close();
	}
}

/**
 *  从客户端接收原始数据
 */
int wTcpTask::RecvBytes(char *vArray, int vLen)
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
			if(iRecvLen < 0 && errno == EINTR)
			{
				continue;
			}
			return iRecvLen;
		}
	}
}

/**
 *  原始发送客户端数据
 */
int wTcpTask::SendBytes(char *vArray, int vLen)
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
			if(errno == EINTR)
			{
				continue;
			}

			LOG_ERROR("default", "SendToClient fd(%d) error: %s", mSocketFD, strerror(errno));
			return iSendLen;
		}
	}
}

/**
 *  接受数据
 *  每条消息大小不能超过100k
 */
int wTcpTask::ListeningRecv()
{
	int iOffset = mRecvBytes;

	int iRecvLen = RecvBytes(mRecvMsgBuff + iOffset, sizeof(mRecvMsgBuff) - iOffset);
	if(iRecvLen <= 0)
	{
		//LOG_NOTICE("default", "client %s socket fd(%d) close from connect port %d, return %d", mIPAddr.c_str(), mSocketFD, mPort, iRecvLen);
		CloseTcpTask(CLOSE_BY_PEER);
		return -1;
	}
	
	mRecvBytes = mRecvBytes + iRecvLen;	//已接受消息总字节数

	char *pBuffer = mRecvMsgBuff;
	int iBuffMsgLen = mRecvBytes;
	
	mStamp = time(NULL);
	int iMsgLen;
	
	//循环处理缓冲区中数据
	while(1)
	{
		if(iBuffMsgLen < sizeof(int))
		{
			break;
		}

		iMsgLen = *(int *)pBuffer;	//消息总长度。不包括自身int:4字节

		//判断消息长度
		if(iMsgLen <= MIN_CLIENT_MSG_LEN || iMsgLen > MAX_CLIENT_MSG_LEN )
		{
			LOG_ERROR("default", "get invalid len %d from %s fd(%d)", iMsgLen, mIPAddr.c_str(), mSocketFD);
			CloseTcpTask(CLOSE_BY_ROUTER);
			return -1;
		}

		iBuffMsgLen -= iMsgLen + sizeof(int);	//buf中除去此条消息剩余数据长度
		pBuffer += iMsgLen + sizeof(int);	//位移到本条消息结束位置地址
		
		//一条不完整消息
		if(iBuffMsgLen < 0)
		{
			//重置buf标识位，待下次循环做准备
			iBuffMsgLen += iMsgLen + sizeof(int);
			pBuffer -= iMsgLen + sizeof(int);
			
#ifdef _DEBUG_
			LOG_DEBUG("default", "fd(%d) recv a part of client msg: real len = %d, now len = %d", mSocketFD, iMsgLen, iBuffMsgLen);
#endif
			break;
		}

#ifdef _DEBUG_
		LOG_DEBUG("default", "get %d bytes msg from %s client fd(%d) to main server", iMsgLen, mIPAddr.c_str(), mSocketFD);
#endif
		//处理逻辑（最好实现成异步）
		int iLen = iMsgLen + sizeof(int);
		HandleRecvMessage(pBuffer - iLen, iLen);
	}

	if(iBuffMsgLen == 0)	//缓冲区无数据
	{
		mRecvBytes = 0;
	}
	else
	{
		//判断剩余的长度
		if((iBuffMsgLen < 0) || (iBuffMsgLen) > MAX_CLIENT_MSG_LEN + sizeof(int))
		{
			LOG_ERROR("default", "the last msg len %d is impossible fd(%d)", iBuffMsgLen, mSocketFD);
			return -1;
		}
		mRecvBytes = iBuffMsgLen;
		memmove(mRecvMsgBuff, pBuffer, iBuffMsgLen);	//清除已处理消息
	}
	
	return 0;
}

int wTcpTask::ListeningSend()
{
	ProcessSendMessage();
}


