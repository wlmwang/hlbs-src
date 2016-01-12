
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
	CMD_REG_DISP(CMD_SVR_RES, SVR_RES_DATA, &AgentCmdTask::SvrResData);
	CMD_REG_DISP(CLI_SVR_REQ, SVR_SET_RES_DATA, &AgentCmdTask::SvrSetResData);
}

int AgentCmdTask::VerifyConn()
{
	//验证登录消息
	char pBuffer[ sizeof(LoginReqToken_t) ];
	int iLen = SyncRecv(pBuffer, sizeof(LoginReqToken_t));
	if (iLen > 0)
	{
		LoginReqToken_t *pLoginRes = (LoginReqToken_t*) pBuffer;
		if (strcmp(pLoginRes->mToken, "Anny") == 0)
		{
			LOG_ERROR("client", "[verify] receive client and verify success from ip(%s) port(%d) with token(%s)", Socket()->IPAddr().c_str(), Socket()->Port(), pLoginRes->mToken);
			mConnType = pLoginRes->mConnType;
			return 0;
		}
		LOG_ERROR("client", "[verify] receive client and verify failed from ip(%s) port(%d) with token(%s)", Socket()->IPAddr().c_str(), Socket()->Port(), pLoginRes->mToken);
	}
	return -1;
}

int AgentCmdTask::Verify()
{
	//验证登录
	LoginReqToken_t stLoginReq;
	stLoginReq.mConnType = SERVER_CMD;
	memcpy(stLoginReq.mToken, "Anny", 4);
	SyncSend((char*)&stLoginReq, sizeof(stLoginReq));
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
			LOG_DEBUG("error", "client fd(%d) send a invalid msg id(%u)", mSocket->SocketFD(), pCommand->GetId());
		}
	}
	return 0;
}

int AgentCmdTask::SvrResData(char *pBuffer, int iLen)
{
	SvrResData_t *pCmd = (struct SvrResData_t* )pBuffer;
	if(pCmd->mNum > 0)
	{
		for(int i = 0; i < pCmd->mNum; i++)
		{
			cout << i << ")";
			cout << " id:" << pCmd->mSvr[i].mId;
			cout << " gid:" << pCmd->mSvr[i].mGid;
			cout << " xid:" << pCmd->mSvr[i].mXid;
			cout << " name:" << pCmd->mSvr[i].mName;
			cout << " ip:" << pCmd->mSvr[i].mIp;
			cout << " port:" << pCmd->mSvr[i].mPort;
			cout << " weight:" << pCmd->mSvr[i].mWeight;
			cout << " disabled:" << pCmd->mSvr[i].mDisabled;
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

int AgentCmdTask::SvrSetResData(char *pBuffer, int iLen)
{
	SvrSetResData_t *pCmd = (struct SvrSetResData_t* )pBuffer;
	if(pCmd->mCode >= 0)
	{
		cout << "send success!" << endl;
	}
	else
	{
		cout << "faild!" << endl;
	}
	AgentCmd::Instance()->SetWaitResStatus(false);
	return 0;
}
