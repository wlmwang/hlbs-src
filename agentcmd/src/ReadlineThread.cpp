
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include <iostream>
#include "ReadlineThread.h"

ReadlineThread::ReadlineThread(char *pPrompt, int iLen)
{
	mReadline.SetPrompt(pPrompt, iLen);
	mReadline.SetCompletion(&ReadlineThread::Completion);
	mCmdLine = 0;
}

ReadlineThread::~ReadlineThread()
{
	//
}

char* ReadlineThread::Generator(const char *pText, int iState)
{
	static int iListIdx = 0, iTextLen = 0;
	if(!iState)
	{
		iListIdx = 0;
		iTextLen = strlen(pText);
	}
	
	const char *pName = NULL;
	/*
	while((pName = AgentCmd::Instance()->GetCmdByIndex(iListIdx)))
	{
		iListIdx++;
		if(!strncmp (pName, pText, iTextLen))
		{
			return strdup(pName);
		}
	}
	*/
	return NULL;
}

char** ReadlineThread::Completion(const char *pText, int iStart, int iEnd)
{
	//rl_attempted_completion_over = 1;
	char **pMatches = NULL;
	if(0 == iStart)
	{
		pMatches = rl_completion_matches(pText, &ReadlineThread::Generator);
	}
	return pMatches;
}

bool ReadlineThread::IsBlocked()
{
	return mCmdLine != 0;
}

int ReadlineThread::PrepareRun()
{
	//...
	return 0;
}

int ReadlineThread::Run()
{
	while(true)
	{
		CondBlock();	//线程控制
		mCmdLine = mReadline.ReadCmdLine();
		if(mReadline.IsUserQuitCmd(mCmdLine))
		{
			StopThread();
			break;
		}
	}
	return 0;
}