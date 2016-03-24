
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include "wTask.h"

wTask::wTask()
{
	Initialize();
}

wTask::wTask(wIO *pIO)
{
	Initialize();
	mIO = pIO;
}

void wTask::Initialize()
{
	mIO = NULL;
	mStatus = TASK_INIT;
	mHeartbeatTimes = 0;
	mRecvBytes = 0;
	mSendBytes = 0;
	mSendWrite = 0;
	memset(&mSendMsgBuff, 0, sizeof(mSendMsgBuff));
	memset(&mRecvMsgBuff, 0, sizeof(mRecvMsgBuff));
}

wTask::~wTask() {}

void wTask::DeleteIO()
{
	SAFE_DELETE(mIO);	//mIO建立在堆上
}

void wTask::CloseTask(int iReason)
{
	mStatus = TASK_QUIT;
	mIO->Close();
}

int wTask::TaskRecv()
{
	int iRecvLen = mIO->RecvBytes(mRecvMsgBuff + mRecvBytes, sizeof(mRecvMsgBuff) - mRecvBytes);
	
	LOG_DEBUG(ELOG_KEY, "[runtime] recv data len: %d , %s", iRecvLen, mRecvMsgBuff);
	if(iRecvLen <= 0)
	{
		return iRecvLen;	
	}
	mRecvBytes += iRecvLen;	

	char *pBuffer = mRecvMsgBuff;	//从头开始读取
	int iBuffMsgLen = mRecvBytes;	//消息总字节数
	int iMsgLen = 0;
	
	while(true)
	{
		if(iBuffMsgLen < sizeof(int))
		{
			break;
		}

		iMsgLen = *(int *)pBuffer;	//完整消息体长度

		//判断消息长度
		if(iMsgLen < MIN_CLIENT_MSG_LEN || iMsgLen > MAX_CLIENT_MSG_LEN )
		{
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
			
			break;
		}
		
		//业务逻辑
		HandleRecvMessage(pBuffer - iMsgLen, iMsgLen);	//去除4字节消息长度标识位
	}

	if(iBuffMsgLen == 0) //缓冲区无数据
	{
		mRecvBytes = 0;
	}
	else
	{
		//判断剩余的长度
		if(iBuffMsgLen < 0)
		{
			return -1;
		}
		
		mRecvBytes = iBuffMsgLen;
		memmove(mRecvMsgBuff, pBuffer, iBuffMsgLen);	//清除已处理消息
	}
	
	return iRecvLen;
}

int wTask::TaskSend()
{
	while(true)
	{
		int iMsgLen = mSendWrite - mSendBytes;
		
		if(iMsgLen <= 0)
		{
			return 0;
		}
		
		int iSendLen = mIO->SendBytes(mSendMsgBuff + mSendBytes, iMsgLen);
		if(iSendLen < 0)
		{
			return iSendLen;
		}
		
		if(iSendLen < iMsgLen)
		{
			mSendBytes += iSendLen;
			continue;
		}
		
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

int wTask::WriteToSendBuf(const char *pCmd, int iLen)
{
	//判断消息长度
	if(iLen <= MIN_CLIENT_MSG_LEN || iLen > MAX_CLIENT_MSG_LEN )
	{
		return -1;
	}
	
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
	
	*(int *)(mSendMsgBuff + mSendWrite)= iLen;
	memcpy(mSendMsgBuff + mSendWrite + sizeof(int), pCmd, iLen);
	return 0;
}

int wTask::SyncSend(const char *pCmd, int iLen)
{
	//判断消息长度
	if(iLen < MIN_CLIENT_MSG_LEN || iLen > MAX_CLIENT_MSG_LEN )
	{
		return -1;
	}
	
	*(int *)mTmpSendMsgBuff = iLen;
	memcpy(mTmpSendMsgBuff + sizeof(int), pCmd, iLen);
	return mIO->SendBytes(mTmpSendMsgBuff, iLen + sizeof(int));
}

int wTask::SyncRecv(char *pCmd, int iLen, int iTimeout)
{
	int iRecvLen = 0, iMsgLen = 0, iTryCount = 0; /*每个消息最多被分为多少个包*/
	long long iSleep = 5000;	//5ms
	struct wCommand* pTmpCmd = 0;
	
	iTryCount = iTimeout * 1000000 / iSleep;
	memset(mTmpRecvMsgBuff, 0, sizeof(mTmpRecvMsgBuff));
	do
	{
		iRecvLen += mIO->RecvBytes(mTmpRecvMsgBuff, iLen + sizeof(int));
		if(iRecvLen <= 0)
		{
			return iRecvLen;	
		}
		if (iRecvLen < iLen + sizeof(int) && iTryCount-- > 0)
		{
			continue;
		}
		
		usleep(iSleep);
		//过滤掉心跳
		pTmpCmd = (struct wCommand*) mTmpRecvMsgBuff;
	} while(pTmpCmd != NULL && pTmpCmd->GetCmd() == CMD_NULL && pTmpCmd->GetPara() == PARA_NULL);
	
	iMsgLen = *(int *)mTmpRecvMsgBuff;
	if(iMsgLen < MIN_CLIENT_MSG_LEN || iMsgLen > MAX_CLIENT_MSG_LEN)
	{
		return -1;
	}

	if (iMsgLen > iRecvLen)	//消息不完整
	{
		return -1;
	}

	if (iMsgLen > iLen)
	{
		return -1;
	}
	memcpy(pCmd, mTmpRecvMsgBuff + sizeof(int), iLen);
	return iRecvLen - sizeof(int);
}
