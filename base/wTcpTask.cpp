
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
#include "wCommand.h"

wTcpTask::wTcpTask(wSocket *pSocket)
{
	Initialize();
	mSocket = pSocket;
}

wTcpTask::wTcpTask()
{
	Initialize();
}

wTcpTask::~wTcpTask()
{
	SAFE_DELETE(mSocket);
}

void wTcpTask::Initialize()
{
	mSocket = NULL;
	mHeartbeatTimes = 0;
	memset(&mSendMsgBuff, 0, sizeof(mSendMsgBuff));
	memset(&mRecvMsgBuff, 0, sizeof(mRecvMsgBuff));
}

/**
 * 清除指定TcpTask
 * @param eReason [关闭原因]
 */
void wTcpTask::CloseTcpTask(int eReason)
{
	if(eReason == 0 || eReason==1) 
	{
		//...
	}
	mSocket->Close();
}

int wTcpTask::Heartbeat()
{
	wCommand* pCmd;
	int iLen = 0;
	char *pBuffer = pCmd->Serialize(iLen);
	int iSendLen = SyncSend(pBuffer,iLen);
	
	SAFE_DELETE(pBuffer);

	mHeartbeatTimes++;
	return -1;
}

/**
 *  处理接受到数据
 *  每条消息大小不能超过100k
 *  核心逻辑：接受整条消息，然后进入用户定义的业务函数HandleRecvMessage
 */
int wTcpTask::ListeningRecv()
{
	int iOffset = mRecvBytes;

	int iRecvLen = mSocket->RecvBytes(mRecvMsgBuff + iOffset, sizeof(mRecvMsgBuff) - iOffset);
	if(iRecvLen <= 0)
	{
		LOG_NOTICE("default", "client %s socket fd(%d) close from connect port %d, return %d", mSocket->IPAddr().c_str(), mSocket->SocketFD(), mSocket->Port(), iRecvLen);
		CloseTcpTask(1);
		return -1;
	}
	
	mRecvBytes = mRecvBytes + iRecvLen;	//已接受消息总字节数

	char *pBuffer = mRecvMsgBuff;
	int iBuffMsgLen = mRecvBytes;
	
	mSocket->Stamp() = time(NULL);
	
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
			LOG_ERROR("default", "get invalid len %d from %s fd(%d)", iMsgLen, mSocket->IPAddr().c_str(), mSocket->SocketFD());
			CloseTcpTask(0);
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
			LOG_DEBUG("default", "fd(%d) recv a part of client msg: real len = %d, now len = %d", mSocket->SocketFD(), iMsgLen, iBuffMsgLen);
#endif
			break;
		}

#ifdef _DEBUG_
		LOG_DEBUG("default", "get %d bytes msg from %s client fd(%d) to main server", iMsgLen, mSocket->IPAddr().c_str(), mSocket->SocketFD());
#endif
		/*start 业务逻辑*/
		//int iLen = iMsgLen + sizeof(int);
		HandleRecvMessage(pBuffer - iMsgLen /*iLen*/, iMsgLen /*iLen*/);
		/*end 业务逻辑*/
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
			LOG_ERROR("default", "the last msg len %d is impossible fd(%d)", iBuffMsgLen, mSocket->SocketFD());
			return -1;
		}
		mRecvBytes = iBuffMsgLen;
		memmove(mRecvMsgBuff, pBuffer, iBuffMsgLen);	//清除已处理消息
	}
	
	return 0;
}

int wTcpTask::SyncSend(char *vArray, int vLen)
{
	return mSocket->SendBytes(vArray, vLen);
}

/**
 *  TODO.
 *  加锁
 */
int wTcpTask::ListeningSend()
{
	return 0;
}

/**
 *  异步发送客户端消息
 *  TODO.
 *  加锁
 */
int wTcpTask::AsyncSend(char *vArray, int vLen)
{
	int iMsgLen = *(int *)vArray;	//消息总长度。
	//判断消息长度
	if(iMsgLen <= MIN_CLIENT_MSG_LEN || iMsgLen > MAX_CLIENT_MSG_LEN )
	{
		LOG_ERROR("default", "message invalid len %d from %s fd(%d)", iMsgLen, mSocket->IPAddr().c_str(), mSocket->SocketFD());
		return -1;
	}
	
	if(sizeof(mSendMsgBuff) - mSendWrite + mSendBytes < iMsgLen + sizeof(int)) //剩余空间不足
	{
		return -1;
	}
	else if(sizeof(mSendMsgBuff) - mSendWrite < iMsgLen + sizeof(int)) //写入空间不足
	{
		memmove(mSendMsgBuff, mSendMsgBuff + mSendBytes, mSendWrite - mSendBytes);	//清除已处理消息
		mSendWrite -= mSendBytes;
		mSendBytes = 0;
	}
	strncpy(mSendMsgBuff + mSendWrite, vArray, vLen);
	return 0;
}
