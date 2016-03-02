
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include "RouterMaster.h"

RouterMaster::RouterMaster()
{
	Initialize();
}

RouterMaster::~RouterMaster()
{
	//
}

void RouterMaster::Initialize()
{
	//
}

void RouterMaster::PrepareRun()
{
	size_t size;
	u_char *p;
	int i;

	RouterConfig *pConfig = RouterConfig::Instance();
    
    //进程标题title="master process " + ngx_argv[0] + ... + ngx_argv[ngx_argc-1]
    size = sizeof(master_process);
    for (i = 0; i < pConfig->mProcTitle->mArgc; i++) 
    {
        size += pConfig->mProcTitle->mArgv[i].size();
    }

    mTitle = new char[size];
    if (mTitle == NULL) 
    {
        exit(1);
    }

    p = (u_char *)memcpy(mTitle, master_process, sizeof(master_process) - 1) + (sizeof(master_process) - 1);

    for (i = 0; i < pConfig->mProcTitle->mArgc; i++) 
    {
        *p++ = ' ';
        p = Cpystrn(p, (u_char *) pConfig->mProcTitle->mArgv[i].c_str(), size);
    }

	pConfig->mProcTitle->Setproctitle(mTitle, "HLFS: ");

	//
}

void RouterMaster::Run()
{
	//
}

wWorker* RouterMaster::NewWorker(int iSlot)
{
	return new RouterWorker();
}