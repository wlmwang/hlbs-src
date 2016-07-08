
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "RouterMaster.h"

RouterMaster::~RouterMaster()
{
    SAFE_DELETE(mTitle);
}

//进程标题 title = "master process " + argv[0] + ... + argv[argc-1]
void RouterMaster::PrepareRun()
{
	size_t size;
	u_char *p;
	int i;
    const char *sProcessTitle = "master process(router)";

    //config、server对象
    mConfig = RouterConfig::Instance();
    if (mConfig == NULL) 
    {
        LOG_ERROR(ELOG_KEY, "[system] RouterConfig instance failed");
        exit(2);
    }
    mServer = RouterServer::Instance();
    if (mServer == NULL) 
    {
        LOG_ERROR(ELOG_KEY, "[system] RouterServer instance failed");
        exit(2);
    }
    ReconfigMaster();

    //进程标题
    size = strlen(sProcessTitle) + 1;
    for (i = 0; i < mConfig->mProcTitle->mArgc; i++) 
    {
        size += strlen(mConfig->mProcTitle->mArgv[i]) + 1;
    }

    mTitle = new char[size];
    p = (u_char *)memcpy(mTitle, sProcessTitle, strlen(sProcessTitle)) + strlen(sProcessTitle);     //前缀。不要\0结尾
    //拼接argv
    for (i = 0; i < mConfig->mProcTitle->mArgc; i++) 
    {
        *p++ = ' ';
        p = Cpystrn(p, (u_char *) mConfig->mProcTitle->mArgv[i], size);
    }
	mConfig->mProcTitle->Setproctitle(mTitle, "HLBS: ");

    mPidFile.FileName() = "/var/run/hlbs_router.pid";   //pid文件名
        
    //准备工作（创建、绑定服务器Listen Socket）
    mServer->PrepareMaster(mConfig->mIPAddr, mConfig->mPort);
}

void RouterMaster::ReconfigMaster()
{
    if (mConfig == NULL) 
    {
        LOG_ERROR(ELOG_KEY, "[system] RouterConfig instance failed");
        return;
    }
    mConfig->GetBaseConf();
    mConfig->GetSvrConf();
    mConfig->GetQosConf();
}

wWorker* RouterMaster::NewWorker(int iSlot)
{
	return new RouterWorker(iSlot);
}