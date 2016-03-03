
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include "AgentMaster.h"

AgentMaster::AgentMaster()
{
	Initialize();
}

AgentMaster::~AgentMaster()
{
	SAFE_DELETE(mConfig);
    SAFE_DELETE(mServer);
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
    const char *sProcessTitle = "master process";

    //config、server对象
    mConfig = AgentConfig::Instance();
    if(mConfig == NULL) 
    {
        LOG_ERROR(ELOG_KEY, "[startup] Get AgentConfig instance failed");
        exit(1);
    }
    mServer = AgentServer::Instance();
    if(mServer == NULL) 
    {
        LOG_ERROR(ELOG_KEY, "[startup] Get AgentServer instance failed");
        exit(1);
    }
    
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

	mConfig->mProcTitle->Setproctitle(mTitle, "HLFS: ");

    //准备工作
    mServer->PrepareStart(mConfig->mIPAddr, mConfig->mPort);

    //pid文件
    //mPidFile.FileName() = LOCK_PATH;
}

void AgentMaster::Run()
{
    //服务器开始运行
    LOG_DEBUG(ELOG_KEY, "[startup] AgentServer start succeed");

    mServer->Start();
}
