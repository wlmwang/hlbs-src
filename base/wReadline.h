
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_READLINE_H_
#define _W_READLINE_H_

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "wType.h"

class wReadline
{
	public:
		typedef char** (*CmdCompletionFunc)(const char *pText, int iStart, int iEnd);

		wReadline();
		~wReadline();
		
		void Initialize();
		
		char *ReadCmdLine();
		char *StripWhite(char *pOrig);
		bool IsUserQuitCmd(char *pCmd);
	
		bool SetCompletionFunc(CmdCompletionFunc pFunc);
	protected:
		char mPrompt[32];
		char *mLineRead;
		char *mStripLine;

		static const char *mQuitCmd[];
		static const unsigned char mQuitCmdNum;

		CmdCompletionFunc mFunc;
};


#endif
