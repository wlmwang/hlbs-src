
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include <iostream>
#include "wAssert.h"
#include "AgentCmd.h"
#include "AgentCmdTask.h"
#include "AgentCmdConfig.h"
#include "LoginCommand.h"

AgentCmdTask::AgentCmdTask()
{
    Initialize();
}

AgentCmdTask::AgentCmdTask(wSocket *pSocket):wTcpTask(pSocket)
{
    Initialize();
}

AgentCmdTask::~AgentCmdTask()
{
    //...
}

void AgentCmdTask::Initialize()
{
	mDispatch.Register("AgentCmdTask", ASSERT_CMD(CMD_RTBL_RES, RTBL_RES_DATA), REG_FUNC_a(ASSERT_CMD(CMD_RTBL_RES, RTBL_RES_DATA), &AgentCmdTask::RtblResData));
	mDispatch.Register("AgentCmdTask", ASSERT_CMD(CMD_RTBL_SET_RES, RTBL_SET_RES_DATA), REG_FUNC_a(ASSERT_CMD(CMD_RTBL_SET_RES, RTBL_SET_RES_DATA), &AgentCmdTask::RtblSetResData));
}

int AgentCmdTask::Verify()
{
	//验证登录
	LoginReqToken_t stLoginRes;
	memcpy(stLoginRes.mToken, "Anny", 4);
	SyncSend((char*)&stLoginRes, sizeof(stLoginRes));
	return 0;
}

/**
 *  接受数据
 */
int AgentCmdTask::HandleRecvMessage(char * pBuffer, int nLen)
{
	W_ASSERT(pBuffer != NULL, return -1);
	//解析消息
	struct wCommand *pCommand = (struct wCommand*) pBuffer;
	return ParseRecvMessage(pCommand , pBuffer, nLen);
}

int AgentCmdTask::ParseRecvMessage(struct wCommand* pCommand, char *pBuffer, int iLen)
{
	if (pCommand->GetId() == ASSERT_CMD(CMD_NULL, PARA_NULL))
	{
		//空消息(心跳返回)
		mHeartbeatTimes = 0;
	}
	else
	{
		auto pF = mDispatch.GetFuncT("AgentCmdTask", pCommand->GetId());

		if (pF != NULL)
		{
			pF->mFunc(pBuffer, iLen);
		}
		else
		{
			LOG_DEBUG("default", "client fd(%d) send a invalid msg id(%u)", mSocket->SocketFD(), pCommand->GetId());
		}
	}
	return 0;
}

int AgentCmdTask::RtblResData(char *pBuffer, int iLen)
{
	RtblResData_t *pCmd = (struct RtblResData_t* )pBuffer;
	if(pCmd->mNum > 0)
	{
		for(int i = 0; i < pCmd->mNum; i++)
		{
			cout << i << ")";
			cout << " id:" << pCmd->mRtbl[i].mId;
			cout << " gid:" << pCmd->mRtbl[i].mGid;
			cout << " xid:" << pCmd->mRtbl[i].mXid;
			cout << " name:" << pCmd->mRtbl[i].mName;
			cout << " ip:" << pCmd->mRtbl[i].mIp;
			cout << " port:" << pCmd->mRtbl[i].mPort;
			cout << " weight:" << pCmd->mRtbl[i].mWeight;
			cout << " disabled:" << pCmd->mRtbl[i].mDisabled;
			cout << endl;
		}
	}
	else
	{
		cout << "empty table." << endl;
	}
	AgentCmd::Instance()->SetWaitResStatus(false);
	return 0;
}

int AgentCmdTask::RtblSetResData(char *pBuffer, int iLen)
{
	RtblSetResData_t *pCmd = (struct RtblSetResData_t* )pBuffer;
	if(pCmd->mRes >= 0)
	{
		cout << "set router table id(" << pCmd->mId <<") success!";
	}
	else
	{
		cout << "faild!" << endl;
	}
	AgentCmd::Instance()->SetWaitResStatus(false);
	return 0;
}
