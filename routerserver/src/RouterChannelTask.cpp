
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include "RouterChannelTask.h"
#include "RouterMaster.h"
#include "wChannelCmd.h"

RouterChannelTask::RouterChannelTask()
{
    Initialize();
}

RouterChannelTask::RouterChannelTask(wIO *pIO) : wChannelTask(pIO)
{
    Initialize();
}

RouterChannelTask::~RouterChannelTask()
{
    //...
}

void RouterChannelTask::Initialize()
{
	CHANNEL_REG_DISP(CMD_CHANNEL_REQ, CHANNEL_REQ_OPEN, &RouterChannelTask::ChannelOpen);
	CHANNEL_REG_DISP(CMD_CHANNEL_REQ, CHANNEL_REQ_CLOSE, &RouterChannelTask::ChannelClose);
	CHANNEL_REG_DISP(CMD_CHANNEL_REQ, CHANNEL_REQ_QUIT, &RouterChannelTask::ChannelQuit);
	CHANNEL_REG_DISP(CMD_CHANNEL_REQ, CHANNEL_REQ_TERMINATE, &RouterChannelTask::ChannelTerminate);
}

/**
 *  接受数据
 */
int RouterChannelTask::HandleRecvMessage(char * pBuffer, int nLen)
{
	W_ASSERT(pBuffer != NULL, return -1);
	//解析消息
	struct wCommand *pCommand = (struct wCommand*) pBuffer;
	return ParseRecvMessage(pCommand, pBuffer, nLen);
}

int RouterChannelTask::ParseRecvMessage(struct wCommand* pCommand, char *pBuffer, int iLen)
{
	if (pCommand->GetId() == CMD_ID(CMD_NULL, PARA_NULL))
	{
		//空消息(心跳返回)
		mHeartbeatTimes = 0;
	}
	else
	{
		LOG_DEBUG(ELOG_KEY, "[runtime] unix message, diapath cmd(%d) parm(%d)", pCommand->GetCmd(), pCommand->GetPara());

		auto pF = mDispatch.GetFuncT("RouterChannelTask", pCommand->GetId());

		if (pF != NULL)
		{
			pF->mFunc(pBuffer, iLen);
		}
		else
		{
			LOG_ERROR("default", "[runtime] client fd(%d) send a invalid msg id(%u)", mIO->FD(), pCommand->GetId());
		}
	}
	return 0;
}

int RouterChannelTask::ChannelOpen(char *pBuffer, int iLen)
{
	RouterMaster *pMaster = RouterMaster::Instance();
	
	ChannelReqOpen_t *pCh = (struct ChannelReqOpen_t* )pBuffer;
	if(pMaster->mWorkerPool != NULL && pMaster->mWorkerPool[pCh->mSlot])
	{	
		LOG_DEBUG(ELOG_KEY, "[runtime] get channel s:%i pid:%d fd:%d",pCh->mSlot, pCh->mPid, pCh->mFD);
		
		pMaster->mWorkerPool[pCh->mSlot]->mPid = pCh->mPid;
		pMaster->mWorkerPool[pCh->mSlot]->mCh[0] = pCh->mFD;
	}
	
	return 0;
}

int RouterChannelTask::ChannelClose(char *pBuffer, int iLen)
{
	RouterMaster *pMaster = RouterMaster::Instance();
	
	ChannelReqClose_t *pCh = (struct ChannelReqClose_t* )pBuffer;
	if(pMaster->mWorkerPool != NULL && pMaster->mWorkerPool[pCh->mSlot])
	{
		LOG_DEBUG(ELOG_KEY, "[runtime] close channel s:%i pid:%P our:%P fd:%d",
				pCh->mSlot, pCh->mPid, pMaster->mWorkerPool[pCh->mSlot]->mPid,
				pMaster->mWorkerPool[pCh->mSlot]->mCh[0]);
		
		if (close(pMaster->mWorkerPool[pCh->mSlot]->mCh[0]) == -1) 
		{
			LOG_DEBUG(ELOG_KEY, "[runtime] close() channel failed: %s", strerror(errno));
		}
		pMaster->mWorkerPool[pCh->mSlot]->mCh[0] = FD_UNKNOWN; //-1
	}
	
	return 0;
}

int RouterChannelTask::ChannelQuit(char *pBuffer, int iLen)
{
	RouterMaster *pMaster = RouterMaster::Instance();
	g_quit = 1;
	return 0;
}

int RouterChannelTask::ChannelTerminate(char *pBuffer, int iLen)
{
	RouterMaster *pMaster = RouterMaster::Instance();
	g_terminate = 1;
	return 0;
}
