
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include <iostream>
#include <unistd.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>

#include "wLog.h"
#include "wTcpTask.h"
#include "wCommand.h"
#include "wMisc.h"

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
	wCommand vCmd;
	int iLen = 0;
	char *pBuffer = Serialize(vCmd, iLen);
	int iSendLen = SyncSend(pBuffer,iLen);
	
	SAFE_DELETE(pBuffer);

	mHeartbeatTimes++;
	return -1;
}

/**
 *  处理接受到数据
 *  每条消息大小[1b,100k)
 *  核心逻辑：接受整条消息，然后进入用户定义的业务函数HandleRecvMessage
 *  return ：<0 对端发生错误|消息超长 =0 对端关闭(FIN_WAIT1) >0 接受字符
 */
int wTcpTask::ListeningRecv()
{
	int iOffset = mRecvBytes;

	int iRecvLen = mSocket->RecvBytes(mRecvMsgBuff + iOffset, sizeof(mRecvMsgBuff) - iOffset);
	if(iRecvLen <= 0)
	{
		LOG_ERROR("default", "client %s socket fd(%d) close from connect port %d, return %d", mSocket->IPAddr().c_str(), mSocket->SocketFD(), mSocket->Port(), iRecvLen);
		return iRecvLen;	
	}
	
	//cout<<"data:"<<mRecvMsgBuff<<"|"<<iRecvLen<<endl;

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
	
	return iRecvLen;
}

/**
 *  TODO.
 *  加锁
 */
int wTcpTask::ListeningSend()
{	
	//循环发送缓冲区中数据
	while(1)
	{
		int iMsgLen = mSendWrite - mSendBytes;
		
		if(iMsgLen <= 0)
		{
			return 0;
		}
		
		int iSendLen = mSocket->SendBytes(mSendMsgBuff + mSendBytes, iMsgLen);
		
		if(iSendLen < 0)
		{
			return iSendLen;
		}
		
		if(iSendLen < iMsgLen)
		{
			mSendBytes += iSendLen;
			continue;
		}
#ifdef _DEBUG_
		LOG_DEBUG("default", "get %d bytes msg from %s client fd(%d) to main server", iMsgLen, mSocket->IPAddr().c_str(), mSocket->SocketFD());
#endif
	}
	
	int iSendLen = mSendBytes;
	if(mSendBytes > 0)
	{
		memmove(mSendMsgBuff, mSendMsgBuff + mSendBytes, mSendWrite - mSendBytes);	//清除已处理消息
		mSendWrite -= mSendBytes;
		mSendBytes = 0;
	}
	return iSendLen;
}

/**
 *  异步发送客户端消息
 *  加锁 TODO.
 *  return 
 *  -1 ：消息长度不合法
 *  -2 ：发送缓冲剩余空间不足，请稍后重试
 *   0 : 发送成功
 */
int wTcpTask::AsyncSend(const char *pCmd, int iLen)
{	
	//判断消息长度
	if(iLen <= MIN_CLIENT_MSG_LEN || iLen > MAX_CLIENT_MSG_LEN )
	{
		LOG_ERROR("default", "message invalid len %d from %s fd(%d)", iLen, mSocket->IPAddr().c_str(), mSocket->SocketFD());
		return -1;
	}
	
	//lock
	int iMsgLen = iLen + sizeof(int);
	if(sizeof(mSendMsgBuff) - mSendWrite + mSendBytes < iMsgLen) //剩余空间不足
	{
		return -2;
	}
	else if(sizeof(mSendMsgBuff) - mSendWrite < iMsgLen) //写入空间不足
	{
		memmove(mSendMsgBuff, mSendMsgBuff + mSendBytes, mSendWrite - mSendBytes);	//清除已处理消息
		mSendWrite -= mSendBytes;
		mSendBytes = 0;
	}

	char sBinaryLen[sizeof(int)];
	itoa(iLen, sBinaryLen, 2);

	strncpy(mSendMsgBuff + mSendWrite, sBinaryLen, sizeof(int));
	strncpy(mSendMsgBuff + mSendWrite + sizeof(int), pCmd, iLen);
	return 0;
}

int wTcpTask::SyncSend(const char *pCmd, int iLen)
{
	//判断消息长度
	if(iLen <= MIN_CLIENT_MSG_LEN || iLen > MAX_CLIENT_MSG_LEN )
	{
		LOG_ERROR("default", "message invalid len %d from %s fd(%d)", iLen, mSocket->IPAddr().c_str(), mSocket->SocketFD());
		return -1;
	}

	char sBinaryLen[sizeof(int)];
	itoa(iLen, sBinaryLen, 2);

	strncpy(mTmpSendMsgBuff, sBinaryLen, sizeof(int));
	strncpy(mTmpSendMsgBuff + sizeof(int), pCmd, iLen);
	
	return mSocket->SendBytes(mTmpSendMsgBuff, iLen + sizeof(int));
}
