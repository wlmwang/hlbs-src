
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_READLINE_H_
#define _W_READLINE_H_

#include <unistd.h>
#include <string.h>
#include <cstdio>

#include <readline/readline.h>
#include <readline/history.h>
#include "wType.h"

class wReadline
{
	public:
		typedef char** (*CmdCompletionFunc_t)(const char *pText, int iStart, int iEnd);	//Tab键能补齐的函数类型

		wReadline();
		~wReadline();
		void Initialize();
		
		char *ReadCmdLine();
		char *StripWhite(char *pOrig);
		bool IsUserQuitCmd(char *pCmd);
	
		bool SetCompletionFunc(CmdCompletionFunc_t pFunc);
	protected:
		char mPrompt[32];
		char *mLineRead;
		char *mStripLine;

		
		CmdCompletionFunc_t mFunc;

		static const char *mQuitCmd[];
		static const unsigned char mQuitCmdNum;
};


#endif
