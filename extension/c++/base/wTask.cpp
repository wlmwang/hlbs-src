
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
	int iSendLen = 0;
	int iMsgLen = 0;
	while(true)
	{
		iMsgLen = mSendWrite - mSendBytes;
		if(iMsgLen <= 0)
		{
			return 0;
		}
		
		iSendLen = mIO->SendBytes(mSendMsgBuff + mSendBytes, iMsgLen);
		if(iSendLen < 0)
		{
			return iSendLen;
		}
		mSendBytes += iSendLen;
		
	}
	
	if(mSendBytes > 0)
	{
		memmove(mSendMsgBuff, mSendMsgBuff + mSendBytes, mSendWrite - mSendBytes);	//清除已处理消息
		mSendWrite -= mSendBytes;
		mSendBytes = 0;
	}
	return mSendBytes;
}

int wTask::SendToBuf(const char *pCmd, int iLen)
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
	mSendWrite += iMsgLen;

	return mSendWrite - mSendBytes;
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
	int iSize = 0, iRecvLen = 0, iMsgLen = 0, iTryCount = 0; /*每个消息最多被分为多少个包*/
	long long iSleep = 5000;	//5ms

	iTryCount = iTimeout*1000000 / iSleep;
	memset(mTmpRecvMsgBuff, 0, sizeof(mTmpRecvMsgBuff));
	
	struct wCommand* pTmpCmd = 0;
	int iCmdMsgLen = sizeof(int) + sizeof(struct wCommand);

	do {
		iSize = mIO->RecvBytes(mTmpRecvMsgBuff + iRecvLen, iLen + sizeof(int));
		if (iSize <= 0)
		{
			break;
		}
		iRecvLen += iSize;

		if ((iRecvLen < iCmdMsgLen) && (iTryCount-- > 0))	//至少一个消息体长度
		{
			usleep(iSleep);
			continue;
		}

		pTmpCmd = (struct wCommand*) (mTmpRecvMsgBuff + sizeof(int));
		if (pTmpCmd != NULL && pTmpCmd->GetCmd() == CMD_NULL && pTmpCmd->GetPara() == PARA_NULL)	//过滤掉心跳
		{
			memmove(mTmpRecvMsgBuff, mTmpRecvMsgBuff + iCmdMsgLen, iRecvLen - iCmdMsgLen);
			iRecvLen -= iCmdMsgLen;
		}
		
		if ((iRecvLen < iLen + sizeof(int)) && (iTryCount-- > 0))
		{
			usleep(iSleep);
			continue;
		}
		break;
	} while(true);
	
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
