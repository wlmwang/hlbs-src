
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_READLINE_H_
#define _W_READLINE_H_

#include <unistd.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

class wReadline
{
	public:
		wReadline();
		~wReadline();
		
		void Initialize();
		
		char *ReadCmdLine();
		char *StripWhite(char *pOrig)
		bool IsUserQuitCmd(char *pCmd);
	
	protected:
		static const char **mQuitCmd;
		static const unsigned char mQuitCmdNum;
			
		char mPrompt[32];
		char *mCmdLine;
		char *mLineRead;
		char *mStripLine;
};

static char* CmdGenerator(const char *pText, int iState)
{
	static int iListIdx = 0, iTextLen = 0;
	if(!iState)
	{
		iListIdx = 0;
		iTextLen = strlen(pText);
	}

	//当输入字符串与命令列表中某命令部分匹配时，返回该命令字符串
	const char *pName = NULL;
	while((pName = GetCmdByIndex(iListIdx)))
	{
		iListIdx++;

		if(!strncmp (pName, pText, iTextLen))
		{
			return strdup(pName);
		}
	}

	return NULL;
}

static char** CmdCompletion(const char *pText, int iStart, int iEnd)
{
	//rl_attempted_completion_over = 1;
	char **pMatches = NULL;
	if(0 == iStart)
	{
		pMatches = rl_completion_matches(pText, CmdGenerator);
	}

	return pMatches;
}

#endif
