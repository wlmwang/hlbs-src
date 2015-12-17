
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include "wReadline.h"

static const char *wReadline::mQuitCmd[] = {"Quit", "Exit", "End", "Bye"};
static const unsigned char wReadline::mQuitCmdNum = sizeof(wReadline::mQuitCmd) / sizeof(wReadline::mQuitCmd[0]);

wReadline::wReadline()
{
	Initialize();
}

wReadline::~wReadline()
{
	//
}

void wReadline::Initialize()
{
	memset(mPrompt,0,32);
	*mCmdLine = 0;
	*mLineRead = 0;
	*mStripLine = 0;
	
	rl_attempted_completion_function = CmdCompletion;
}


char *wReadline::ReadCmdLine()
{
	SAFE_FREE(mLineRead);
	mLineRead = readline(mPrompt);

	//ÌÞ³ýÃüÁîÐÐÊ×Î²µÄ¿Õ°××Ö·û¡£ÈôÌÞ³ýºóµÄÃüÁî²»Îª¿Õ£¬Ôò´æÈëÀúÊ·ÁÐ±í
	mStripLine = StripWhite(mLineRead);
	if(mStripLine && *mStripLine)
	{
		add_history(mStripLine);
	}

	return mStripLine;
}

//ÌÞ³ý×Ö·û´®Ê×Î²µÄ¿Õ°××Ö·û(º¬¿Õ¸ñ)
char *wReadline::StripWhite(char *pOrig)
{
	if(NULL == pOrig)
		return NULL;

	char *pStripHead = pOrig;
	while(isspace(*pStripHead))
	{
		pStripHead++;
	}
	if('\0' == *pStripHead)
	{
		return pStripHead;
	}

	char *pStripTail = pStripHead + strlen(pStripHead) - 1;
	while(pStripTail > pStripHead && isspace(*pStripTail))
	{
		pStripTail--;
	}
	
	*(++pStripTail) = '\0';

	return pStripHead;
}

bool wReadline::IsUserQuitCmd(char *pCmd)
{
	for(unsigned char ucQuitCmdIdx = 0; ucQuitCmdIdx < wReadline::mQuitCmdNum; ucQuitCmdIdx++)
	{
		if(!strcasecmp(pCmd, wReadline::mQuitCmd[ucQuitCmdIdx]))
		{
			return true;
		}
	}

	return false;
}

