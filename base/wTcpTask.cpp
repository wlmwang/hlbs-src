
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
	mRecvBytes = 0;
	mSendBytes = 0;
	mSendWrite = 0;
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
	vCmd.mConnType = ConnType();

	SyncSend((char*)&vCmd, sizeof(vCmd));

	mHeartbeatTimes++;
	return -1;
}

int wTcpTask::VerifyConn()
{
	return 0;
}

int wTcpTask::Verify()
{
	return 0;
}

/**
 *  处理接受到数据
 *  每条消息大小[2b,128k)
 *  核心逻辑：接受整条消息，然后进入用户定义的业务函数HandleRecvMessage
 *  return ：<0 对端发生错误|消息超长 =0 对端关闭(FIN_WAIT1) >0 接受字符
 */
int wTcpTask::ListeningRecv()
{
	int iOffset = mRecvBytes;

	int iRecvLen = mSocket->RecvBytes(mRecvMsgBuff + iOffset, sizeof(mRecvMsgBuff) - iOffset);
	
	//cout<< "data:" <<mRecvMsgBuff<< "|" <<iRecvLen<<endl;
	
	if(iRecvLen <= 0)
	{
		LOG_ERROR("default", "client %s socket fd(%d) close from connect port %d, return %d", mSocket->IPAddr().c_str(), mSocket->SocketFD(), mSocket->Port(), iRecvLen);
		return iRecvLen;	
	}

	mRecvBytes = mRecvBytes + iRecvLen;	

	char *pBuffer = mRecvMsgBuff;	//从头开始读取
	int iBuffMsgLen = mRecvBytes;	//消息总字节数
	
	int iMsgLen = 0;
	
	while(1)
	{
		if(iBuffMsgLen < sizeof(int))
		{
			break;
		}

		iMsgLen = *(int *)pBuffer;	//完整消息体长度

		//判断消息长度
		if(iMsgLen < MIN_CLIENT_MSG_LEN || iMsgLen > MAX_CLIENT_MSG_LEN )
		{
			LOG_ERROR("default", "get invalid len %d from %s fd(%d)", iMsgLen, mSocket->IPAddr().c_str(), mSocket->SocketFD());
			return -1;
		}

		iBuffMsgLen -= iMsgLen + sizeof(int);	//buf中除去当前完整消息剩余数据长度
		pBuffer += iMsgLen + sizeof(int);		//位移到本条消息结束位置地址
		
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
		HandleRecvMessage(pBuffer - iMsgLen, iMsgLen);	//去除4字节消息长度标识位
		/*end 业务逻辑*/
	}

	if(iBuffMsgLen == 0) //缓冲区无数据
	{
		mRecvBytes = 0;
		//memset(mRecvMsgBuff, 0, sizeof(mRecvMsgBuff));
	}
	else
	{
		//判断剩余的长度
		if(iBuffMsgLen < 0)
		{
			LOG_ERROR("default", "the last msg len %d is impossible fd(%d)", iBuffMsgLen, mSocket->SocketFD());
			return -1;
		}
		mRecvBytes = iBuffMsgLen;
		memmove(mRecvMsgBuff, pBuffer, iBuffMsgLen);	//清除已处理消息
	}
	
	return iRecvLen;
}

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
 *  return 
 *  -1 ：消息长度不合法
 *  -2 ：发送缓冲剩余空间不足，请稍后重试
 *   0 : 发送成功
 */
int wTcpTask::WriteToSendBuf(const char *pCmd, int iLen)
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
	
	//暂时方案，可用结构体off_t设置TODO
	struct wCommand* pTmpCmd = (struct wCommand*) pCmd;
	pTmpCmd->mConnType = ConnType();

	*(int *)(mSendMsgBuff + mSendWrite)= iLen;
	memcpy(mSendMsgBuff + mSendWrite + sizeof(int), pCmd, iLen);
	return 0;
}

//同步发送确切长度消息
int wTcpTask::SyncSend(const char *pCmd, int iLen)
{
	//memset(mTmpSendMsgBuff, 0, sizeof(mTmpSendMsgBuff));
	//判断消息长度
	if(iLen < MIN_CLIENT_MSG_LEN || iLen > MAX_CLIENT_MSG_LEN )
	{
		LOG_ERROR("default", "message invalid len %d from %s fd(%d)", iLen, mSocket->IPAddr().c_str(), mSocket->SocketFD());
		return -1;
	}
	
	//暂时方案，可用结构体off_t设置TODO
	struct wCommand* pTmpCmd = (struct wCommand*) pCmd;
	pTmpCmd->mConnType = ConnType();

	*(int *)mTmpSendMsgBuff = iLen;
	memcpy(mTmpSendMsgBuff + sizeof(int), pCmd, iLen);
	return mSocket->SendBytes(mTmpSendMsgBuff, iLen + sizeof(int));
}

/**
 *  最好在设置为阻塞模式下启用该函数，毕竟只有超时了(30s)或者接受不完整消息才出错
 *  确保pCmd有足够长的空间接受自此同步消息
 */
int wTcpTask::SyncRecv(char *pCmd, int iLen)
{
	int iRecvLen = 0, iMsgLen = 0;
	struct wCommand* pTmpCmd = 0;
	
	memset(mTmpRecvMsgBuff, 0, sizeof(mTmpRecvMsgBuff));
	do
	{
		iRecvLen = mSocket->RecvBytes(mTmpRecvMsgBuff, iLen + sizeof(int));
		if(iRecvLen <= 0)
		{
			LOG_ERROR("default", "client %s socket fd(%d) close from connect port %d, return %d", mSocket->IPAddr().c_str(), mSocket->SocketFD(), mSocket->Port(), iRecvLen);
			return iRecvLen;	
		}
		//过滤掉心跳
		pTmpCmd = (struct wCommand*) mTmpRecvMsgBuff;
		if(pTmpCmd->GetCmd() != CMD_NULL && pTmpCmd->GetPara() != PARA_NULL)
		{
			break;
		}
	} while(true);
	
	iMsgLen = *(int *)mTmpRecvMsgBuff;
	if(iMsgLen < MIN_CLIENT_MSG_LEN || iMsgLen > MAX_CLIENT_MSG_LEN)
	{
		LOG_ERROR("default", "get invalid len %d from %s fd(%d)", iMsgLen, mSocket->IPAddr().c_str(), mSocket->SocketFD());
		return -1;
	}

	if (iMsgLen > iRecvLen)	//消息不完整
	{
		LOG_DEBUG("default", "fd(%d) recv a part of client msg: real len = %d, now len = %d", mSocket->SocketFD(), iMsgLen, iRecvLen);
		return -1;
	}

	if (iMsgLen > iLen)
	{
		LOG_DEBUG("default", "error buffer len, it\'s to short!");
		return -1;
	}
	memcpy(pCmd, mTmpRecvMsgBuff + sizeof(int), iLen);
	return iRecvLen - sizeof(int);
}
