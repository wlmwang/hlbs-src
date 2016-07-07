
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "AgentMaster.h"

AgentMaster::AgentMaster()
{
	Initialize();
}

AgentMaster::~AgentMaster()
{
    SAFE_DELETE(mTitle);
}

void AgentMaster::Initialize()
{
	mTitle = NULL;
    mConfig = NULL;
    mServer = NULL;
}

//进程标题 title = "master process " + argv[0] + ... + argv[argc-1]
void AgentMaster::PrepareRun()
{
	size_t size;
	u_char *p;
	int i;
    const char *sProcessTitle = "master process(agent)";

    //获取config、server对象（在main中已初始化完成）
    mConfig = AgentConfig::Instance();
    if (mConfig == NULL) 
    {
        LOG_ERROR(ELOG_KEY, "[system] Get AgentConfig instance failed");
        exit(1);
    }
    mServer = AgentServer::Instance();
    if (mServer == NULL) 
    {
        LOG_ERROR(ELOG_KEY, "[system] Get AgentServer instance failed");
        exit(1);
    }

    //进程标题
    size = strlen(sProcessTitle) + 1;
    for (i = 0; i < mConfig->mProcTitle->mArgc; i++) 
    {
        size += strlen(mConfig->mProcTitle->mArgv[i]) + 1;
    }
    mTitle = new char[size];
    if (mTitle == NULL) 
    {
        exit(1);
    }

    p = (u_char *)memcpy(mTitle, sProcessTitle, strlen(sProcessTitle)) + strlen(sProcessTitle);     //不要\0结尾

    for (i = 0; i < mConfig->mProcTitle->mArgc; i++) 
    {
        *p++ = ' ';
        p = Cpystrn(p, (u_char *) mConfig->mProcTitle->mArgv[i], size);
    }

	mConfig->mProcTitle->Setproctitle(mTitle, "HLBS: ");   //设置标题

    //mPidFile.FileName() = "../log/hlbs.pid";    //进程pid文件
    mPidFile.FileName() = "/var/run/hlbs_agent.pid";

    mServer->PrepareSingle(mConfig->mIPAddr, mConfig->mPort);    //初始化服务器
}

void AgentMaster::Run()
{
    //服务器开始运行
    mServer->SingleStart();
}
