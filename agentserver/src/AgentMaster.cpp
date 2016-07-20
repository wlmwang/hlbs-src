
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "Common.h"
#include "AgentMaster.h"

AgentMaster::~AgentMaster()
{
    SAFE_DELETE(mTitle);
}

//进程标题 title = "master process " + argv[0] + ... + argv[argc-1]
void AgentMaster::PrepareRun()
{
    //获取config、server对象（在main中已初始化完成）
    mConfig = AgentConfig::Instance();
    if (mConfig == NULL) 
    {
        LOG_ERROR(ELOG_KEY, "[system] AgentConfig instance failed");
        exit(0);
    }
    mServer = AgentServer::Instance();
    if (mServer == NULL) 
    {
        LOG_ERROR(ELOG_KEY, "[system] AgentServer instance failed");
        exit(0);
    }

    const char *sProcessTitle = "master process(agent)";
    size_t size = strlen(sProcessTitle) + 1;
    for (int i = 0; i < mConfig->mProcTitle->mArgc; i++) 
    {
        size += strlen(mConfig->mProcTitle->mArgv[i]) + 1;
    }

    mTitle = new char[size];
    u_char *ptr = (u_char *)memcpy(mTitle, sProcessTitle, strlen(sProcessTitle)) + strlen(sProcessTitle);     //前缀。不要\0结尾
    for (int i = 0; i < mConfig->mProcTitle->mArgc; i++) 
    {
        *ptr++ = ' ';
        ptr = Cpystrn(ptr, (u_char *) mConfig->mProcTitle->mArgv[i], size);
        //ptr = Cpystrn(ptr, (u_char *) mConfig->mProcTitle->mArgv[i], strlen(mConfig->mProcTitle->mArgv[i]));    //不要\0结尾
    }
    //*ptr = '\0';
    mConfig->mProcTitle->Setproctitle(mTitle, "HLBS: ");

    mPidFile.FileName() = AGENT_PID_FILE;

    mServer->PrepareSingle(mConfig->mIPAddr, mConfig->mPort);    //初始化服务器
}

void AgentMaster::Run()
{
    //服务器开始运行
    mServer->SingleStart();
}
