
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "Common.h"
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
    size_t size = strlen(sProcessTitle) + 1;
    for (size_t i = 0; i < mConfig->mProcTitle->mArgc; i++) 
    {
        size += strlen(mConfig->mProcTitle->mArgv[i]) + 1;
    }
    mTitle = new char[size];

    u_char *ptr = (u_char *)memcpy(mTitle, sProcessTitle, strlen(sProcessTitle)) + strlen(sProcessTitle);     //不要\0结尾
    for (size_t i = 0; i < mConfig->mProcTitle->mArgc; i++) 
    {
        *ptr++ = ' ';
        //ptr = Cpystrn(ptr, (u_char *) mConfig->mProcTitle->mArgv[i], size);
        ptr = Cpystrn(ptr, (u_char *) mConfig->mProcTitle->mArgv[i], strlen(mConfig->mProcTitle->mArgv[i]));    //不要\0结尾
    }
    *ptr = '\0';
	mConfig->mProcTitle->Setproctitle(mTitle, "HLBS: ");   //设置标题

    mPidFile.FileName() = AGENT_PID_FILE;

    mServer->PrepareSingle(mConfig->mIPAddr, mConfig->mPort);    //初始化服务器
}

void AgentMaster::Run()
{
    //服务器开始运行
    mServer->SingleStart();
}
