
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include "wMProcess.h"

wMProcess::wMProcess()
{
	mPid = getpid();
	mName = "mater process";
}

wMProcess::~wMProcess()
{
	//...
}

void wMProcess::Start(int iNum)
{
	mShm = new wShareMemory();
	mShm->CreateShareMemory(mName.c_str());

	pid_t pid;
	for(int i = 0; i < iNum; i++)
	{
		pid = GenerateWorker(i, "worker process");

		if (G_pProc != NULL && G_pProc->mPid > 0 && pid == G_pProc->mPid)
		{
			mProcList.insert(pair<int, wProcess*>(G_pProc->mPid, G_pProc));
		}
	}
}

wProcess* wMProcess::NewProcess(int iIdx, string sTitle)
{
	wProcess* pProc = new wProcess(sTitle);

	if(pProc->mShm->AttachShareMemory(sTitle.c_str()) == 0)
	{
		SAFE_DELETE(pProc);
		return 0;
	}
	return pProc;
}

pid_t wMProcess::GenerateWorker(int iIdx, string sTitle)
{
	pid_t pid;

	G_pProc = NewProcess(iIdx, sTitle);
	if (G_pProc == 0)
	{
		return -2;
	}

	pid = fork();
    switch (pid) 
    {
	    case -1:
	    	LOG_ERROR("default", "fork() failed while GenerateWorker \"%s\": %s", sTitle.c_str(), strerror(errno));
	    	SAFE_DELETE(G_pProc);
	        return -1;
	    case 0:
	        G_pProc->Start();
	        break;

	    default:
	        break;
    }

    return pid;
}